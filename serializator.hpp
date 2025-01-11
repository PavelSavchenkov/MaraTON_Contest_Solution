#pragma once

#include <random>

#include "utils.h"
#include "compressor.hpp"
#include "huffman.hpp"
#include "lz_compressor_bits.hpp"

namespace Serializator {
const std::string compressed_dict = "";

std::basic_string<uint8_t> uncompress_dict() {
    return {};

    td::Slice slice(compressed_dict.data(), compressed_dict.size());
    std::string dict_bytes_str = td::base64_decode(slice).move_as_ok();
    std::basic_string<uint8_t> dict_bytes(reinterpret_cast<uint8_t *>(dict_bytes_str.data()), dict_bytes_str.size());
    const auto final_dict = lz_compressor_bits::decompress(dict_bytes);
    // std::cout << "final_dict.size() = " << final_dict.size() << std::endl;
    return final_dict;
}

const std::basic_string<uint8_t> uncompressed_dict = uncompress_dict();

static const uint8_t BITS_DATA_SIZE = 10;

struct MyCell {
    unsigned cnt_refs{-1u};
    std::array<unsigned, 4> refs{};
    std::array<td::Ref<vm::Cell>, 4> refs_as_cells{};

    bool is_special{};

    std::basic_string<uint8_t> data{};
    unsigned cnt_bits{-1u};

    CellType get_type() const {
        if (!is_special) {
            return CellType::Ordinary;
        }
        const auto type = static_cast<CellType>((data.at(0) >> 6) + 1);
        CHECK(type != CellType::Ordinary);
        return type;
    }

    MyCell() = default;

    explicit MyCell(td::Ref<vm::Cell> cell) {
        vm::Cell::LoadedCell cell_loaded = cell->load_cell().move_as_ok();
        CellType cell_type = cell_loaded.data_cell->special_type();
        is_special = (cell_type != CellType::Ordinary);
        cnt_refs = cell_loaded.data_cell->get_refs_cnt();

        cnt_bits = cell_loaded.data_cell->size();

        vm::CellSlice cell_slice = cell_to_slice(cell);

        std::basic_string<uint8_t> data_buf((cnt_bits + 7) / 8, 0);
        td::BitPtr data_bit_buf(data_buf.data(), 0);

        unsigned bits_left = cnt_bits;
        while (bits_left > 0) {
            uint8_t cur_bits = std::min(bits_left, 8u);
            uint8_t byte = cell_slice.fetch_ulong(cur_bits);
            bits_left -= cur_bits;
            if (is_special && data_bit_buf.offs == 0) {
                CHECK(cur_bits >= 8);
                CHECK(byte != 0);
                --byte;
                CHECK(byte < 4);
                data_bit_buf.store_uint(byte, 2);
                data_bit_buf.offs += 2;
                cnt_bits -= 6;
            } else {
                data_bit_buf.store_uint(byte, cur_bits);
                data_bit_buf.offs += cur_bits;
            }
        }
        data = std::basic_string<uint8_t>(data_bit_buf.ptr, (cnt_bits + 7) / 8);

        const unsigned cnt_refs = cell_loaded.data_cell->get_refs_cnt();
        CHECK(cnt_refs <= 4);
        for (unsigned i = 0; i < cnt_refs; i++) {
            td::Ref<vm::Cell> ref = cell_loaded.data_cell->get_ref(i);
            refs_as_cells[i] = ref;
        }

        if (cell_type == CellType::MerkleUpdate || cell_type == CellType::MerkleProof) {
            data.resize(1);
            cnt_bits = 2;
        }
    }

    uint8_t d1() const {
        return cnt_refs + 8 * is_special;
    }

    uint8_t d2() const {
        unsigned desc = cnt_bits / 8;
        desc *= 2;
        if (cnt_bits % 8 != 0) {
            desc ^= 1u;
        }
        return desc;
    }

    bool operator==(const MyCell& rhs) const {
        if (cnt_bits != rhs.cnt_bits)
            return false;
        if (data != rhs.data)
            return false;
        if (is_special != rhs.is_special)
            return false;
        if (cnt_refs != rhs.cnt_refs)
            return false;
        if (refs != rhs.refs)
            return false;
        return true;
    }

    // void print() const {
    //     std::cout << "MyCell|cnt_bits=" << cnt_bits;
    //     std::cout << ", is_special=" << is_special << ", cnt_refs=" << cnt_refs;
    //     std::cout << ", data=";
    //     for (const auto &c: data) {
    //         std::cout << std::bitset<8>(c) << " ";
    //     }
    //     std::cout << std::dec;
    //     std::cout << ", refs=";
    //     for (const auto &r: refs) {
    //         std::cout << r << " ";
    //     }
    //     std::cout << "|" << std::endl;
    // }
};

std::vector<Ptr<MyCell> > convert_to_my_cells(td::Ref<vm::Cell> root) {
    std::vector<Ptr<MyCell> > my_cells;

    std::unordered_map<const vm::Cell *, unsigned> pos;
    auto dfs = [&](auto&& self, td::Ref<vm::Cell> cell) -> void {
        if (pos.count(cell.get())) {
            return;
        }
        pos[cell.get()] = my_cells.size();
        auto my_cell = std::make_shared<MyCell>(cell);
        my_cells.push_back(my_cell);
        CHECK(my_cell->cnt_refs <= 4);
        for (unsigned i = 0; i < my_cell->cnt_refs; ++i) {
            self(self, my_cell->refs_as_cells[i]);
        }
    };

    dfs(dfs, root);

    for (const auto& cell: my_cells) {
        for (unsigned i = 0; i < cell->cnt_refs; ++i) {
            td::Ref<vm::Cell> ref = cell->refs_as_cells[i];
            cell->refs[i] = pos.at(ref.get());
        }
    }

    return my_cells;
}

std::basic_string<uint8_t> serialize(td::Ref<vm::Cell> root) {
    auto my_cells = convert_to_my_cells(root);
    const unsigned n = my_cells.size();

    // reorder cells
    {
        std::vector<uint8_t> out_degree(n);
        std::vector<std::vector<unsigned> > who_refers(n);
        for (unsigned i = 0; i < n; i++) {
            out_degree[i] = my_cells[i]->cnt_refs;
            for (unsigned j = 0; j < my_cells[i]->cnt_refs; j++) {
                who_refers[my_cells[i]->refs[j]].push_back(i);
            }
        }

        auto cmp = [&](unsigned i, unsigned j) {
            if (out_degree[i] != out_degree[j]) {
                return out_degree[i] < out_degree[j];
            }
            if (who_refers[i].size() != who_refers[j].size()) {
                return who_refers[i].size() > who_refers[j].size();
            }
            return i < j;
        };

        std::set<unsigned, decltype(cmp)> cells(cmp);
        for (unsigned i = 0; i < n; i++) {
            cells.insert(i);
        }

        std::vector<unsigned> order;
        order.reserve(n);
        while (!cells.empty()) {
            auto it = cells.begin();
            unsigned i = *it;
            cells.erase(it);

            CHECK(out_degree[i] == 0);

            order.push_back(i);

            for (unsigned from: who_refers[i]) {
                cells.erase(from);
                CHECK(out_degree[from] > 0);
                --out_degree[from];
                cells.insert(from);
            }
        }

        // SHUFFLE
        // std::random_device rd;
        // std::mt19937 g(rd());
        // std::shuffle(order.begin(), order.end(), g);
        // std::sort(order.begin(), order.end(), [&](unsigned i, unsigned j) {
        //     return my_cells[i]->cnt_bits < my_cells[j]->cnt_bits;
        // });

        // update ref indexes
        std::vector<unsigned> pos(n);
        for (unsigned i = 0; i < n; i++) {
            pos[order[i]] = i;
        }

        for (auto& my_cell: my_cells) {
            for (unsigned i = 0; i < my_cell->cnt_refs; i++) {
                my_cell->refs[i] = pos[my_cell->refs[i]];
            }
        }

        // physically reorder cells
        {
            std::vector<Ptr<MyCell> > new_my_cells(n);
            for (unsigned i = 0; i < n; i++) {
                new_my_cells[pos[i]] = my_cells[i];
            }
            my_cells.swap(new_my_cells);
        }
    }

    std::basic_string<uint8_t> bytes(50 + n + n * 4 * 3 + n * 256, 0);
    td::BitPtr bits_ptr(bytes.data(), 0);
    huffman::HuffmanEncoderDecoder d1_encoder(huffman::d1_freq);
    huffman::HuffmanEncoderDecoder data_size_encoder(huffman::data_size_freq);
    std::basic_string<unsigned> cell_id;

    // store n
    bits_ptr.store_uint(n, 20);
    bits_ptr.offs += 20;

    // compress refs
    const bool compress_refs = false;
    bits_ptr.store_uint(compress_refs, 1);
    bits_ptr.offs += 1;

    // d1
    // auto old = bits_ptr.offs;
    for (unsigned i = 0; i < n; ++i) {
        const auto d1 = my_cells[i]->d1();
        auto spent_bits = d1_encoder.encode_symbol(bits_ptr, d1);
        cell_id += std::basic_string<unsigned>(spent_bits, i);
    }
    // std::cout << "d1 len = " << bits_ptr.offs - old << std::endl;

    // data sizes
    // old = bits_ptr.offs;
    for (unsigned i = 0; i < n; ++i) {
        const auto data_size = my_cells[i]->cnt_bits;
        auto spent_bits = data_size_encoder.encode_symbol(bits_ptr, data_size);
        cell_id += std::basic_string<unsigned>(spent_bits, i);
    }
    // std::cout << "data sizes len = " << bits_ptr.offs - old << std::endl;

    // refs
    // old = bits_ptr.offs;
    for (unsigned i = 0; i < n; ++i) {
        const auto bit_len = compress_refs ? len_in_bits(i) : len_in_bits(n - 1);
        for (unsigned j = 0; j < my_cells[i]->cnt_refs; ++j) {
            const unsigned ref = my_cells[i]->refs[j];
            // CHECK(ref < i);

            const auto ref_store = (n + i - ref) % n; //rd();
            bits_ptr.store_uint(ref_store, bit_len);
            bits_ptr.offs += bit_len;

            cell_id += std::basic_string<unsigned>(bit_len, i);
        }
    }
    // std::cout << "refs len = " << bits_ptr.offs - old << std::endl;

    const unsigned data_start = bits_ptr.offs;
    // data
    for (unsigned i = 0; i < n; ++i) {
        const auto my_cell = my_cells[i];
        td::ConstBitPtr data_ptr(my_cell->data.data(), 0);
        for (unsigned j = 0; j < my_cell->cnt_bits; ++j) {
            bool bit = data_ptr.get_uint(1);
            data_ptr.offs += 1;
            bits_ptr.store_uint(bit, 1);
            bits_ptr.offs += 1;
        }
        cell_id += std::basic_string<unsigned>(my_cell->cnt_bits, i);
    }
    const unsigned data_end = bits_ptr.offs;
    // std::cout << "data len = " << data_end - data_start << std::endl;

    const auto output = std::basic_string<uint8_t>(bits_ptr.ptr, (bits_ptr.offs + 7) / 8);

    // check lz references
    // if (false) {
    //     const auto is_son = [&](auto&& self, const unsigned par, const unsigned son) -> bool {
    //         for (unsigned i = 0; i < my_cells[par]->cnt_refs; ++i) {
    //             const auto ref = my_cells[par]->refs[i];
    //             if (ref == son || self(self, ref, son)) {
    //                 return true;
    //             }
    //         }
    //         return false;
    //     };
    //
    //     const auto bits = bytes_str_to_bit_str(output);
    //     const unsigned n = bits.size();
    //     std::vector<unsigned> best_match_suff(n, -1);
    //     std::vector<unsigned> best_match_len(n, 0);
    //     // suff aut
    //     {
    //         SuffAut<2> suff_aut_bits(bits);
    //         suff_aut_bits.build_matches(
    //             best_match_suff,
    //             best_match_len,
    //             lz_compressor_bits::MIN_MATCH_LENGTH,
    //             -1u
    //         );
    //     }
    //
    //     std::map<unsigned, unsigned> cnt_with_matches_x_cells;
    //     std::map<unsigned, unsigned> sum_len_matches;
    //     std::set<unsigned> ids_involved;
    //     std::set<unsigned> ids_referenced;
    //     std::set<std::basic_string<uint8_t> > match_prefs;
    //     const unsigned pref_th = 64;
    //     unsigned cnt_matches = 0;
    //     std::map<unsigned, unsigned> cell_offsets;
    //     unsigned cnt_interleaved_matches = 0;
    //     unsigned sum_matches_len = 0;
    //     unsigned cnt_matches_with_sons = 0;
    //     DSU dsu(my_cells.size());
    //     unsigned cnt_wallet_cells_who_reference = 0;
    //     unsigned cnt_wallet_cells_who_reference_in_data = 0;
    //     unsigned cnt_wallet_cells_who_reference_other_wallet_cells = 0;
    //     for (unsigned i = 0; i < n;) {
    //         const auto match_len = best_match_len[i];
    //         if (match_len >= lz_compressor_bits::MIN_MATCH_LENGTH) {
    //             const auto start = best_match_suff[i];
    //             if (start + match_len > i) {
    //                 ++cnt_interleaved_matches;
    //             }
    //             if (my_cells[cell_id[i]]->cnt_bits == 282) {
    //                 ++cnt_wallet_cells_who_reference;
    //                 if (data_start <= i && i < data_end) {
    //                     ++cnt_wallet_cells_who_reference_in_data;
    //                 }
    //             }
    //             std::set<unsigned> diff_ids;
    //             bool match_son = false;
    //             for (unsigned j = 0; j < match_len; ++j) {
    //                 const auto id_ref = cell_id[start + j];
    //                 if (is_son(is_son, cell_id[i], id_ref)) {
    //                     match_son = true;
    //                 }
    //                 dsu.unite(cell_id[i], id_ref);
    //                 diff_ids.insert(id_ref);
    //                 CHECK(bits[start + j] == bits[i + j]);
    //             }
    //             if (my_cells[cell_id[i]]->cnt_bits == 282) {
    //                 bool ref_other_wallet = false;
    //                 for (auto id: diff_ids) {
    //                     if (my_cells[id]->cnt_bits == 282) {
    //                         ref_other_wallet = true;
    //                     }
    //                 }
    //                 cnt_wallet_cells_who_reference_other_wallet_cells += ref_other_wallet;
    //             }
    //             cnt_matches_with_sons += match_son;
    //             unsigned cell_offset = 0;
    //             while (start >= cell_offset && cell_id[start] == cell_id[start - cell_offset]) {
    //                 ++cell_offset;
    //             }
    //             cell_offsets[cell_offset] += 1;
    //             cnt_with_matches_x_cells[diff_ids.size()] += 1;
    //             sum_len_matches[diff_ids.size()] += match_len;
    //
    //             const unsigned pref_len = std::min(pref_th, match_len);
    //             match_prefs.insert(bits.substr(start, pref_len));
    //
    //             ids_involved.insert(cell_id[i]);
    //             for (auto id: diff_ids) {
    //                 ids_involved.insert(id);
    //                 ids_referenced.insert(id);
    //             }
    //             cnt_matches += 1;
    //
    //             sum_matches_len += match_len;
    //
    //             i += match_len;
    //         } else {
    //             ++i;
    //         }
    //     }
    //     unsigned sum_len_referenced_cells = 0;
    //     for (const auto id: ids_referenced) {
    //         if (id != -1u) {
    //             sum_len_referenced_cells += my_cells[id]->cnt_bits;
    //         }
    //     }
    //     std::cout << "stats for MATCHING other cells: " << std::endl;
    //     for (const auto& it: cnt_with_matches_x_cells) {
    //         std::cout << it.second << " matches referenced " << it.first << " different other cells, "
    //                 << "avg len is " << sum_len_matches[it.first] * 1.0 / it.second << std::endl;
    //     }
    //     std::cout << ids_involved.size() << " cells involved in matches out of "
    //             << my_cells.size() << " cells" << std::endl;
    //     std::cout << "there are " << match_prefs.size() << " different match prefixes of length "
    //             << pref_th << " out of " << cnt_matches << " matches" << std::endl;
    //     std::cout << cell_offsets.size() << " different cell offsets (up to ~1023), out of "
    //             << cnt_matches << " matches" << std::endl;
    //     std::cout << "cnt_interleaved_matches = " << cnt_interleaved_matches << " out of "
    //             << cnt_matches << " matches" << std::endl;
    //     std::cout << "sum_matches_len = " << sum_matches_len << ", this is "
    //             << (sum_matches_len * 100.0 / n) << "% out of all bits" << std::endl;
    //     std::cout << "sum_len_referenced_cells = " << sum_len_referenced_cells << std::endl;
    //     std::cout << "cnt referenced cells = " << ids_referenced.size() << std::endl;
    //     std::cout << "cnt_matches_with_sons = " << cnt_matches_with_sons << std::endl;
    //     std::cout << "cnt connected comps of cells by refs = " << dsu.cnt_comps() << std::endl;
    //     std::cout << "comp sizes: " << std::endl;
    //     std::map<unsigned, unsigned> cnt_with_sz;
    //     for (const auto sz: dsu.comp_sizes()) {
    //         cnt_with_sz[sz] += 1;
    //     }
    //     for (const auto& it: cnt_with_sz) {
    //         std::cout << it.second << " comps with sz=" << it.first << std::endl;
    //     }
    //     std::cout << "serialized len = " << n << std::endl;
    //
    //     // for (auto it : cell_offsets) {
    //     //     std::cout << "cell_offset=" << it.first << ", its cnt=" << it.second << std::endl;
    //     // }
    //     std::cout << "cnt_wallet_cells_who_reference = " << cnt_wallet_cells_who_reference << std::endl;
    //     std::cout << "cnt_wallet_cells_who_reference_in_data = " << cnt_wallet_cells_who_reference_in_data << std::endl;
    //     std::cout << "cnt_wallet_cells_who_reference_other_wallet_cells = "
    //             << cnt_wallet_cells_who_reference_other_wallet_cells << std::endl;
    //
    //     std::map<unsigned, unsigned> cnt_with_data_sz;
    //     for (const auto& cell: my_cells) {
    //         cnt_with_data_sz[cell->cnt_bits] += 1;
    //         // if (cell->cnt_bits == 282) {
    //         //     td::BitPtr ptr(cell->data.data(), 0);
    //         //     std::cout << std::hex << "0x" << ptr.get_uint(4 * 8) << std::dec << std::endl;
    //         // }
    //     }
    //     // std::cout << "static const std::vector<unsigned> data_size_freq = {";
    //     // for (const auto &it: cnt_with_data_sz) {
    //     //     std::cout << it << ",";
    //     // }
    //     // std::cout << "};" << std::endl;
    //     // exit(0);
    //     // for (const auto& it: cnt_with_data_sz) {
    //     //     std::cout << it.second << " cells with data sz=" << it.first << std::endl;
    //     // }
    //     // exit(0);
    // }

    return output;
}

td::BufferSlice serialize_slice(td::Ref<vm::Cell> root) {
    auto bytes = serialize(root);
    return td::BufferSlice(reinterpret_cast<const char *>(bytes.data()), bytes.size());
}

td::Ref<vm::Cell> deserialise(std::basic_string<uint8_t> buffer) {
    td::BitPtr buffer_bit(buffer.data(), 0);
    huffman::HuffmanEncoderDecoder d1_decoder(huffman::d1_freq);
    huffman::HuffmanEncoderDecoder data_size_decoder(huffman::data_size_freq);

    const unsigned n = buffer_bit.get_uint(20);
    buffer_bit.offs += 20;

    const bool compress_refs = buffer_bit.get_uint(1);
    buffer_bit.offs += 1;

    std::vector<Ptr<MyCell> > my_cells;
    my_cells.reserve(n);

    // build my_cells
    for (unsigned i = 0; i < n; ++i) {
        Ptr<MyCell> my_cell = std::make_shared<MyCell>();

        const auto d1 = d1_decoder.decode_symbol(buffer_bit);
        my_cell->cnt_refs = d1 & 7;
        CHECK(my_cell->cnt_refs <= 4);
        my_cell->is_special = d1 & 8;

        my_cells.push_back(my_cell);
    }

    // data sizes
    for (auto& my_cell: my_cells) {
        const auto data_size = data_size_decoder.decode_symbol(buffer_bit);
        my_cell->cnt_bits = data_size;
    }

    // read refs
    for (unsigned i = 0; i < n; ++i) {
        auto& my_cell = my_cells[i];

        const auto bit_len = compress_refs ? len_in_bits(i) : len_in_bits(n - 1);
        for (unsigned j = 0; j < my_cell->cnt_refs; j++) {
            unsigned ref_id = (n + i - buffer_bit.get_uint(bit_len)) % n;
            buffer_bit.offs += bit_len;
            my_cell->refs[j] = ref_id;
            if (compress_refs) {
                CHECK(ref_id < i);
            }
        }
    }

    for (unsigned i = 0; i < n; ++i) {
        auto& my_cell = my_cells[i];
        my_cell->data.resize((my_cell->cnt_bits + 7) / 8, 0);
        td::BitPtr my_cell_bit_ptr(my_cell->data.data(), 0);
        for (unsigned j = 0; j < my_cell->cnt_bits; ++j) {
            my_cell_bit_ptr.store_uint(buffer_bit.get_uint(1), 1);
            buffer_bit.offs += 1;
            my_cell_bit_ptr.offs += 1;
        }
    }

    // top sort, just in case
    std::vector<unsigned> order; {
        std::vector<char> was(n, 0);
        const auto dfs = [&](auto&& self, unsigned v) -> void {
            CHECK(v < n);
            CHECK(!was[v]);
            was[v] = true;
            for (unsigned i = 0; i < my_cells[v]->cnt_refs; ++i) {
                const auto to = my_cells[v]->refs[i];
                if (!was[to]) {
                    self(self, to);
                }
            }
            order.push_back(v);
        };
        for (unsigned v = 0; v < n; ++v) {
            if (!was[v]) {
                dfs(dfs, v);
            }
        }
    }
    CHECK(order.size() == n);

    // build DAG of td::Ref<vm::Cell>
    std::vector<td::Ref<vm::Cell> > cells(n);
    std::vector<char> ready(n, 0);
    for (const auto i: order) {
        const auto& my_cell = my_cells.at(i);

        vm::CellBuilder cell_builder;

        // add refs
        std::vector<td::Ref<vm::Cell> > refs;
        for (unsigned j = 0; j < my_cell->cnt_refs; ++j) {
            unsigned ref_id = my_cell->refs[j];
            // CHECK(len_in_bits(ref_id) <= len_in_bits(i));
            CHECK(cells[ref_id].get());
            CHECK(ready[ref_id]);
            cell_builder.store_ref(cells[ref_id]);
            refs.push_back(cells[ref_id]);
        }

        const auto cell_type = my_cell->get_type();
        std::basic_string<uint8_t> data = my_cell->data;;
        unsigned cnt_bits = my_cell->cnt_bits;
        if (my_cell->is_special) {
            const uint8_t cell_type_int = (data[0] >> 6) + 1;
            cnt_bits += 6;

            std::basic_string<uint8_t> new_data((cnt_bits + 7) / 8, 0);
            td::BitPtr new_data_ptr(new_data.data(), 0);
            new_data_ptr.store_uint(cell_type_int, 8);
            new_data_ptr.offs += 8;

            td::BitPtr data_ptr(data.data(), 2);
            unsigned new_data_bits = new_data_ptr.offs;
            push_bit_range(new_data, new_data_bits, data_ptr, 2, cnt_bits - 8);

            data = new_data;
            CHECK(new_data_bits == cnt_bits);
            cnt_bits = new_data_bits;
        }
        if (cell_type == CellType::MerkleUpdate || cell_type == CellType::MerkleProof) {
            // hash0 (256), hash1, depth0 (8 * bytes_for_index), depth1
            CHECK(refs.size() <= 2);
            CHECK(data.size() == 1);
            CHECK(cnt_bits == 8);
            const uint8_t bytes_for_index_max = len_in_bytes(n);

            data.resize(1 + 256 / 8 * 2 + 8 * bytes_for_index_max, 0);
            td::BitPtr data_ptr(data.data(), 8);

            for (unsigned j = 0; j < refs.size(); ++j) {
                const auto hash = refs[j]->get_hash(0);
                const auto slice = hash.as_slice();
                for (unsigned k = 0; k < 256 / 8; ++k) {
                    data_ptr.store_uint(slice.data()[k], 8);
                    data_ptr.offs += 8;
                }
            }

            for (unsigned j = 0; j < refs.size(); ++j) {
                unsigned depth = refs[j]->get_depth(0);
                data_ptr.store_uint(depth, bytes_for_index_max * 8);
                data_ptr.offs += bytes_for_index_max * 8;
            }
            cnt_bits = data_ptr.offs;
        }

        // add data
        cell_builder.store_bits(
            data.data(),
            cnt_bits
        );

        td::Ref<vm::DataCell> cell{};
        auto finalised = cell_builder.finalize_novm_nothrow(my_cell->is_special);
        cell = finalised.move_as_ok();
        CHECK(cell->is_special() == my_cell->is_special);
        CHECK(cell->get_bits() == cnt_bits);

        cells[i] = cell;
        ready[i] = true;
    }

    return cells[order.back()];
}

td::Ref<vm::Cell> deserialise_slice(td::Slice buffer_slice) {
    std::basic_string<uint8_t> buffer(buffer_slice.data(), buffer_slice.data() + buffer_slice.size());
    return deserialise(buffer);
}

std::string compress(
    const std::string& base64_data,
    bool return_before_huffman = false,
    bool return_after_serialize = false
) {
    CHECK(!base64_data.empty());
    td::BufferSlice data(td::base64_decode(base64_data).move_as_ok());
    td::Ref<vm::Cell> root = vm::std_boc_deserialize(data).move_as_ok();

    auto S = serialize(root);
    if (return_after_serialize) {
        return to_string(S);
    }
    // CHECK(!uncompressed_dict.empty());
    S = lz_compressor_bits::compress(S, uncompressed_dict);

    if (return_before_huffman) {
        return to_string(S);
    }
    S = huffman::encode_8(S);
    auto base64 = td::base64_encode(td::Slice(S.data(), S.size()));
    return base64;
}

std::string decompress(const std::string& base64_data) {
    CHECK(!base64_data.empty());
    std::string data = td::base64_decode(base64_data).move_as_ok();
    std::basic_string<uint8_t> S(data.begin(), data.end());
    S = huffman::decode_8(S);

    S = lz_compressor_bits::decompress(S, uncompressed_dict);

    td::Ref<vm::Cell> root = deserialise(S);
    td::BufferSlice serialised_properly = vm::std_boc_serialize(root, 31).move_as_ok();
    return base64_encode(serialised_properly);
}
};
