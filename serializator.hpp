#pragma once

#include <random>

#include "utils.h"
#include "huffman.hpp"
#include "suff_aut.hpp"

namespace Serializator {
static const unsigned MIN_MATCH_LEN = 32;
static const uint8_t BITS_DATA_SIZE = 10;
static const uint8_t BITS_FOR_N_CELLS = 20;
static const uint8_t BITS_TO_DUMP_LITERAL_LEN = 9;
static const uint8_t BITS_TO_DUMP_MATCH_LEN = 8;
static const uint8_t BITS_FOR_WITHIN_CELL_OFFSET_CURPLUS = 4;

static const unsigned LITERAL_LEN_TH = 500;
static const unsigned CELL_ID_OFFSET_TH = 1000;
static const unsigned MATCH_LEN_TH = 300;
static const unsigned OFFSET_WITHIN_CELL_TH = 1047;


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

enum class FieldType : uint8_t {
    none      = 0,
    d1        = 1,
    data_size = 2,
    refs      = 3,
    data      = 4
};

struct SerializeMetaData {
    unsigned cell_id{-1u};
    FieldType field_type{FieldType::none};

    bool operator ==(const SerializeMetaData& m) const {
        return cell_id == m.cell_id && field_type == m.field_type;
    }
};

inline std::ostream& operator <<(std::ostream& s, const SerializeMetaData& m) {
    s << "meta: {cell_id=" << m.cell_id << ", type=";
    switch (m.field_type) {
        case FieldType::none: {
            s << "none";
            break;
        }
        case FieldType::d1: {
            s << "d1";
            break;
        }
        case FieldType::data_size: {
            s << "data_size";
            break;
        }
        case FieldType::refs: {
            s << "refs";
            break;
        }
        case FieldType::data: {
            s << "data";
            break;
        }
        default:
            CHECK(false);
    }
    s << "}";
    return s;
}

void build_stats(
    const std::basic_string<uint8_t>& bits,
    const std::vector<SerializeMetaData>& meta
) {
    const unsigned n = bits.size();
    SuffAut<2> suff_aut_bits(bits);
    const auto matches = suff_aut_bits.build_matches(
        MIN_MATCH_LEN
    );
    CHECK(n == matches.size());

    unsigned n_cells = 0;
    for (const auto& m: meta) {
        n_cells = std::max(n_cells, m.cell_id + 1);
    }

    // offsets within cells
    if (true) {
        std::map<unsigned, unsigned> seen_bits_for_cell;
        std::vector<unsigned> cell_offset(n, -1u);
        for (unsigned i = 0; i < n; ++i) {
            const auto& m = meta[i];
            if (m.cell_id == -1u) {
                continue;
            }
            cell_offset[i] = seen_bits_for_cell[m.cell_id];
            seen_bits_for_cell[m.cell_id] += 1;
        }

        static unsigned max_offset = 0;
        static std::map<unsigned, unsigned> offset_cnt;
        // for (unsigned i = 0; i < n; ++i) {
        //     for (const auto& match : matches[i]) {
        //         const auto s = match.start;
        //         const auto off = cell_offset[s];
        //         if (off != -1u) {
        //             offset_cnt[off] += 1;
        //             max_offset = std::max(max_offset, off);
        //         }
        //     }
        // }
        for (unsigned i = 0; i < n;) {
            unsigned max_len = 0;
            unsigned best_off = -1u;
            for (const auto& match: matches[i]) {
                const auto s = match.start;
                const auto len = match.len;
                const auto off = cell_offset[s];
                if (len > max_len) {
                    max_len = len;
                    best_off = off;
                }
            }
            if (best_off != -1u) {
                best_off = std::min(OFFSET_WITHIN_CELL_TH, best_off);
                offset_cnt[best_off] += 1;
                max_offset = std::max(max_offset, best_off);
                i += max_len;
            } else {
                ++i;
            }
        }
        for (unsigned off = 0; off <= max_offset; ++off) {
            offset_cnt[off] += 0;
        }


        std::cout << "offset_within_cell_cnt.size() = " << offset_cnt.size() << std::endl;
        std::cout << "static const std::vector<unsigned> offset_within_cell_freq = {";
        for (unsigned i = 0; i < offset_cnt.size(); ++i) {
            std::cout << offset_cnt[i];
            if (i + 1 < offset_cnt.size()) {
                std::cout << ", ";
            }
        }
        std::cout << "};" << std::endl;
        std::cout << std::endl;
    }

    // offsets of cells themselves, data only
    if (false) {
        static std::map<unsigned, unsigned> offset_cnt;
        static unsigned max_offset = 0;

        for (unsigned i = 0; i < n;) {
            if (meta[i].cell_id == -1u) {
                ++i;
                continue;
            }
            unsigned max_id = -1u;
            unsigned max_len = 0;
            for (const auto& match: matches[i]) {
                const auto s = match.start;
                const auto len = match.len;
                const auto id = meta[s].cell_id;
                if (len > max_len && id != -1u && meta[s].field_type == FieldType::data) {
                    // std::cout << "id=" << id << ", meta[i].cell_id=" << meta[i].cell_id << std::endl;
                    CHECK(id <= meta[i].cell_id);
                    max_id = id;
                    max_len = len;
                }
            }
            if (max_id != -1u) {
                const auto off = meta[i].cell_id - max_id;
                offset_cnt[off] += 1;
                max_offset = std::max(max_offset, off);
                i += max_len;
            } else {
                ++i;
            }
        }

        for (unsigned off = 0; off <= max_offset; ++off) {
            offset_cnt[off] += 0;
        }

        std::cout << "offset_cells.size() = " << offset_cnt.size() << std::endl;
        // std::cout << "const std::vector<unsigned> offset_cnt = {";
        for (unsigned i = 0; i < offset_cnt.size(); ++i) {
            std::cout << offset_cnt[i];
            if (i + 1 < offset_cnt.size()) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
        // std::cout << "}" << std::endl;
    }

    // literal lengths
    if (false) {
        static std::map<unsigned, unsigned> len_cnt;
        static unsigned global_max_len = 0;

        unsigned last_literal_start = 0;
        for (unsigned i = 0; i < n;) {
            unsigned max_len = 0;
            for (const auto& match: matches[i]) {
                const auto s = match.start;
                const auto len = match.len;
                if (len > max_len) {
                    max_len = len;
                }
            }
            if (max_len > 0) {
                const auto literal_len = std::min(i - last_literal_start, LITERAL_LEN_TH);
                global_max_len = std::max(global_max_len, literal_len);
                len_cnt[literal_len] += 1;
                i += max_len;
                last_literal_start = i;
            } else {
                ++i;
            }
        }
        if (last_literal_start < n) {
            len_cnt[n - last_literal_start] += 1;
        }

        for (unsigned len = 0; len <= global_max_len; ++len) {
            len_cnt[len] += 0;
        }

        std::cout << "literal_lengths_freq.size() = " << len_cnt.size() << std::endl;
        std::cout << "static const std::vector<unsigned> literal_lengths_freq = {";
        for (unsigned i = 0; i < len_cnt.size(); ++i) {
            std::cout << len_cnt[i];
            if (i + 1 < len_cnt.size()) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
        std::cout << "};" << std::endl;
    }

    // match lengths
    if (false) {
        static std::map<unsigned, unsigned> len_cnt;
        static unsigned global_max_len = 0;

        for (unsigned i = 0; i < n;) {
            unsigned max_len = 0;
            for (const auto& match: matches[i]) {
                const auto s = match.start;
                const auto len = match.len;
                if (len > max_len) {
                    max_len = len;
                }
            }
            if (max_len > 0) {
                max_len = std::min(max_len, MATCH_LEN_TH);
                global_max_len = std::max(global_max_len, max_len);
                len_cnt[max_len] += 1;
                i += max_len;
            } else {
                ++i;
            }
        }

        for (unsigned len = 0; len <= global_max_len; ++len) {
            len_cnt[len] += 0;
        }

        std::cout << "match_lengths.size() = " << len_cnt.size() << std::endl;
        std::cout << "static const std::vector<unsigned> match_len_freq = {";
        for (unsigned i = 0; i < len_cnt.size(); ++i) {
            std::cout << len_cnt[i];
            if (i + 1 < len_cnt.size()) {
                std::cout << ", ";
            }
        }
        // std::cout << std::endl;
        std::cout << "};" << std::endl;
    }

    // offsets of cells themselves, all matches
    if (false) {
        static std::map<unsigned, unsigned> offset_cnt;
        static unsigned max_offset = 0;

        for (unsigned i = 0; i < n;) {
            if (meta[i].cell_id == -1u) {
                ++i;
                continue;
            }
            unsigned max_id = -1u;
            unsigned max_len = 0;
            for (const auto& match: matches[i]) {
                const auto s = match.start;
                const auto len = match.len;
                const auto id = meta[s].cell_id;
                if (len > max_len && id != -1u) {
                    max_id = id;
                    max_len = len;
                }
            }
            if (max_id != -1u) {
                auto off = (n_cells + meta[i].cell_id - max_id) % n_cells;
                off = std::min(off, CELL_ID_OFFSET_TH);
                offset_cnt[off] += 1;
                max_offset = std::max(max_offset, off);
                i += max_len;
            } else {
                ++i;
            }
        }

        for (unsigned off = 0; off <= max_offset; ++off) {
            offset_cnt[off] += 0;
        }

        std::cout << "offset_cells_all_matches.size() = " << offset_cnt.size() << std::endl;
        std::cout << "static const std::vector<unsigned> cell_id_offset_freq = {";
        for (unsigned i = 0; i < offset_cnt.size(); ++i) {
            std::cout << offset_cnt[i];
            if (i + 1 < offset_cnt.size()) {
                std::cout << ", ";
            }
        }
        std::cout << "};" << std::endl;
        std::cout << std::endl;
    }
}

inline void apply_order(
    std::vector<Ptr<MyCell> >& my_cells,
    const std::vector<unsigned>& order
) {
    const unsigned n_cells = my_cells.size();
    CHECK(order.size() == n_cells);

    std::vector<unsigned> pos(n_cells);
    for (unsigned i = 0; i < n_cells; i++) {
        pos[order[i]] = i;
    }

    // update ref indexes
    for (auto& my_cell: my_cells) {
        for (unsigned i = 0; i < my_cell->cnt_refs; i++) {
            my_cell->refs[i] = pos[my_cell->refs[i]];
        }
    }

    // physically reorder cells
    std::vector<Ptr<MyCell> > new_my_cells(n_cells);
    for (unsigned i = 0; i < n_cells; i++) {
        new_my_cells[pos[i]] = my_cells[i];
    }
    my_cells.swap(new_my_cells);
}

inline std::vector<unsigned> initial_cell_order(const std::vector<Ptr<MyCell> >& my_cells) {
    const unsigned n_cells = my_cells.size();

    std::vector<uint8_t> out_degree(n_cells);
    std::vector<std::vector<unsigned> > who_refers(n_cells);
    for (unsigned i = 0; i < n_cells; i++) {
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
    for (unsigned i = 0; i < n_cells; i++) {
        cells.insert(i);
    }

    std::vector<unsigned> order;
    order.reserve(n_cells);
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
    // std::stable_sort(
    //     order.begin(),
    //     order.end(),
    //     [&](unsigned i, unsigned j) {
    //         const auto& ci = my_cells[i];
    //         const auto& cj = my_cells[j];
    //         if (ci->cnt_bits != cj->cnt_bits) {
    //             return ci->cnt_bits > cj->cnt_bits;
    //         }
    //         if (ci->data != cj->data) {
    //             return ci->data < cj->data;
    //         }
    //         if (ci->d1() != cj->d1()) {
    //             return ci->d1() > cj->d1();
    //         }
    //         return false;
    //     }
    // );

    return order;
}

std::basic_string<uint8_t> my_cells_to_bit_str(
    const std::vector<Ptr<MyCell> >& my_cells,
    std::vector<SerializeMetaData>& meta
) {
    const unsigned n_cells = my_cells.size();

    std::basic_string<uint8_t> bytes(50 + n_cells + n_cells * 4 * 3 + n_cells * 256, 0);
    td::BitPtr bits_ptr(bytes.data(), 0);
    huffman::HuffmanEncoderDecoder d1_encoder(huffman::d1_freq);
    huffman::HuffmanEncoderDecoder data_size_encoder(huffman::data_size_freq);

    const auto push_d1 = [&](const unsigned i) {
        const auto d1 = my_cells[i]->d1();
        auto spent_bits = d1_encoder.encode_symbol(bits_ptr, d1);
        for (unsigned j = 0; j < spent_bits; ++j) {
            meta.push_back({i, FieldType::d1});
        }
    };

    const auto push_data_size = [&](const unsigned i) {
        const auto data_size = my_cells[i]->cnt_bits;
        auto spent_bits = data_size_encoder.encode_symbol(bits_ptr, data_size);
        for (unsigned j = 0; j < spent_bits; ++j) {
            meta.push_back({i, FieldType::data_size});
        }
    };

    const auto push_refs = [&](const unsigned i) {
        const auto bit_len = len_in_bits(n_cells - 1);
        for (unsigned j = 0; j < my_cells[i]->cnt_refs; ++j) {
            const unsigned ref = my_cells[i]->refs[j];
            CHECK(ref != i);
            CHECK(((n_cells + i - ref) % n_cells) > 0);

            const auto ref_store = ((n_cells + i - ref) % n_cells) - 1; // 1 ... n-1
            bits_ptr.store_uint(ref_store, bit_len);
            bits_ptr.offs += bit_len;

            for (unsigned j = 0; j < bit_len; ++j) {
                meta.push_back({i, FieldType::refs});
            }
        }
    };

    const auto push_data = [&](const unsigned i) {
        const auto my_cell = my_cells[i];
        td::ConstBitPtr data_ptr(my_cell->data.data(), 0);
        for (unsigned j = 0; j < my_cell->cnt_bits; ++j) {
            bool bit = data_ptr.get_uint(1);
            data_ptr.offs += 1;
            bits_ptr.store_uint(bit, 1);
            bits_ptr.offs += 1;
        }
        for (unsigned j = 0; j < my_cell->cnt_bits; ++j) {
            meta.push_back({i, FieldType::data});
        }
    };

    // d1
    for (unsigned i = 0; i < n_cells; ++i) {
        push_d1(i);
    }
    for (unsigned i = 0; i < n_cells; ++i) {
        push_data_size(i);
    }
    for (unsigned i = 0; i < n_cells; ++i) {
        push_refs(i);
    }
    // data
    for (unsigned i = 0; i < n_cells; ++i) {
        push_data(i);
    }
    bits_ptr.store_uint(1, 1);
    bits_ptr.offs += 1;

    const auto output = std::basic_string<uint8_t>(bits_ptr.ptr, (bits_ptr.offs + 7) / 8);
    return output;
}

std::vector<unsigned> build_offsets_within_cells(const std::vector<SerializeMetaData>& meta) {
    const unsigned n = meta.size();
    std::map<unsigned, unsigned> seen_bits_for_cell;
    std::vector<unsigned> cell_offset(n, -1u);
    for (unsigned i = 0; i < n; ++i) {
        const auto& m = meta[i];
        if (m.cell_id == -1u) {
            continue;
        }
        cell_offset[i] = seen_bits_for_cell[m.cell_id];
        seen_bits_for_cell[m.cell_id] += 1;
    }
    return cell_offset;
}

std::basic_string<uint8_t> run_lz(
    const std::basic_string<uint8_t>& bits,
    const std::vector<SerializeMetaData>& meta
) {
    const unsigned n = bits.size();
    SuffAut<2> suff_aut_bits(bits);
    const auto matches = suff_aut_bits.build_matches(
        MIN_MATCH_LEN
    );
    CHECK(n == matches.size());

    unsigned n_cells = 0;
    for (const auto& m: meta) {
        n_cells = std::max(n_cells, m.cell_id + 1);
    }

    const auto within_cell_offset = build_offsets_within_cells(meta);

    std::basic_string<uint8_t> output_bytes((n / 8 + 1) * 4 / 3, 0);
    td::BitPtr bit_ptr(output_bytes.data(), 0);

    // store n_cells
    bit_ptr.store_uint(n_cells, BITS_FOR_N_CELLS);
    bit_ptr.offs += BITS_FOR_N_CELLS;

    const auto dump_num = [&](unsigned num, const uint8_t blen) {
        const unsigned mx = (1u << blen) - 1;
        while (true) {
            const unsigned cur = std::min(num, mx);
            bit_ptr.store_uint(cur, blen);
            bit_ptr.offs += blen;
            num -= cur;
            if (cur < mx) {
                break;
            }
        }
    };

    huffman::HuffmanEncoderDecoder literal_len_encoder(huffman::literal_lengths_freq);
    huffman::HuffmanEncoderDecoder match_len_encoder(huffman::match_len_freq);
    huffman::HuffmanEncoderDecoder cell_id_offset_encoder(huffman::cell_id_offset_freq);
    huffman::HuffmanEncoderDecoder offset_within_cell_encoder(huffman::offset_within_cell_freq);
    const auto dump_literal = [&](const unsigned literal_len, const unsigned start) {
        // std::cout << "encode: literal_len=" << literal_len << std::endl;

        // literal len
        const auto literal_len_symbol = std::min(LITERAL_LEN_TH, literal_len);
        literal_len_encoder.encode_symbol(bit_ptr, literal_len_symbol);
        if (literal_len_symbol == LITERAL_LEN_TH) {
            dump_num(literal_len - LITERAL_LEN_TH, BITS_TO_DUMP_LITERAL_LEN);
        }

        // literal
        for (unsigned j = 0; j < literal_len; ++j) {
            const auto bit = bits[start + j];
            bit_ptr.store_uint(bit, 1);
            bit_ptr.offs += 1;
        }
    };
    unsigned last_literal_start = 0;
    for (unsigned i = 0; i < n;) {
        unsigned match_len = 0;
        unsigned best_s = -1u;
        if (meta[i].cell_id != -1u) {
            for (const auto& m: matches[i]) {
                auto l = m.len;
                auto s = m.start;
                if (meta[s].cell_id != -1u && l > match_len) {
                    match_len = l;
                    best_s = s;
                }
            }
        }
        if (best_s != -1u) {
            const auto literal_len = i - last_literal_start;
            dump_literal(literal_len, last_literal_start);

            // match len
            const auto match_len_symbol = std::min(MATCH_LEN_TH, match_len);
            match_len_encoder.encode_symbol(bit_ptr, match_len_symbol);
            if (match_len_symbol == MATCH_LEN_TH) {
                dump_num(match_len - MATCH_LEN_TH, BITS_TO_DUMP_MATCH_LEN);
            }

            // match position
            // offset of cell id
            const auto ref_cell_id = meta[best_s].cell_id;
            CHECK(ref_cell_id != -1u);
            const auto cur_cell_id = meta[i].cell_id;
            const auto cell_id_offset = (n_cells + cur_cell_id - ref_cell_id) % n_cells;
            const auto cell_id_offset_symbol = std::min(cell_id_offset, CELL_ID_OFFSET_TH);
            cell_id_offset_encoder.encode_symbol(bit_ptr, cell_id_offset_symbol);
            if (cell_id_offset_symbol == CELL_ID_OFFSET_TH) {
                const auto blen = len_in_bits((n_cells - 1) - CELL_ID_OFFSET_TH);
                bit_ptr.store_uint(cell_id_offset - CELL_ID_OFFSET_TH, blen);
                bit_ptr.offs += blen;
            }
            // debug

            // offset within cell
            const auto offset_within_cell = within_cell_offset[best_s];
            const auto offset_within_cell_symbol = std::min(offset_within_cell, OFFSET_WITHIN_CELL_TH);
            offset_within_cell_encoder.encode_symbol(bit_ptr, offset_within_cell_symbol);
            if (offset_within_cell_symbol == OFFSET_WITHIN_CELL_TH) {
                dump_num(offset_within_cell - OFFSET_WITHIN_CELL_TH, BITS_FOR_WITHIN_CELL_OFFSET_CURPLUS);
            }

            // update state
            i += match_len;
            last_literal_start = i;
        } else {
            ++i;
        }
    }
    if (last_literal_start < n) {
        dump_literal(n - last_literal_start, last_literal_start);
    }
    bit_ptr.store_uint(1, 1);
    bit_ptr.offs += 1;

    const std::basic_string<uint8_t> output(bit_ptr.ptr, static_cast<size_t>(bit_ptr.offs + 7) / 8);
    return output;
}

std::basic_string<uint8_t> serialize(td::Ref<vm::Cell> root) {
    auto my_cells = convert_to_my_cells(root);

    const auto order = initial_cell_order(my_cells);
    apply_order(my_cells, order);

    std::vector<SerializeMetaData> meta;
    const auto output = my_cells_to_bit_str(my_cells, meta);

    const auto output_expanded = bytes_str_to_bit_str(output, true);
    CHECK(output_expanded.size() == meta.size());

    //build_stats(output_expanded, meta);

    const auto lz_output = run_lz(output_expanded, meta);

    return lz_output;
}

inline unsigned read_dumped_num(td::BitPtr& ptr, const uint8_t blen) {
    unsigned num = 0;
    const unsigned mx = (1u << blen) - 1;
    while (true) {
        const auto cur = ptr.get_uint(blen);
        ptr.offs += blen;
        num += cur;
        if (cur < mx) {
            break;
        }
    }
    return num;
}

td::Ref<vm::Cell> my_cells_to_vm_root(const std::vector<Ptr<MyCell> >& my_cells) {
    const auto n_cells = my_cells.size();
    std::vector<unsigned> order;
    // top sort, just in case
    {
        std::vector<char> was(n_cells, 0);
        const auto dfs = [&](auto&& self, unsigned v) -> void {
            CHECK(v < n_cells);
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
        for (unsigned v = 0; v < n_cells; ++v) {
            if (!was[v]) {
                dfs(dfs, v);
            }
        }
    }
    CHECK(order.size() == n_cells);

    // build DAG of td::Ref<vm::Cell>
    std::vector<td::Ref<vm::Cell> > cells(n_cells);
    std::vector<char> ready(n_cells, 0);
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
            const uint8_t bytes_for_index_max = len_in_bytes(n_cells);

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

struct CellMeta {
    unsigned d1_start{-1u};
    unsigned d1_cnt{};

    unsigned data_size_start{-1u};
    unsigned data_size_cnt{};

    unsigned refs_start{-1u};
    unsigned refs_cnt{};

    unsigned data_start{-1u};
    unsigned data_cnt{};

    unsigned get_pos(unsigned offset) const {
        if (offset < d1_cnt) {
            return d1_start + offset;
        }
        offset -= d1_cnt;

        if (offset < data_size_cnt) {
            return data_size_start + offset;
        }
        offset -= data_size_cnt;

        if (offset < refs_cnt) {
            return refs_start + offset;
        }
        offset -= refs_cnt;

        CHECK(offset < data_cnt);
        return data_start + offset;
    }
};

td::Ref<vm::Cell> deserialise(std::basic_string<uint8_t> lz_bytes) {
    td::BitPtr lz_bits(lz_bytes.data(), 0);
    const unsigned lz_cnt_bits = lz_bytes.size() * 8 - (td::count_trailing_zeroes32(lz_bytes.back()) + 1);

    std::basic_string<uint8_t> out_bytes(lz_bytes.size() * 2, 0);
    td::BitPtr out_bits(out_bytes.data(), 0);
    unsigned out_bits_write_ptr = 0;

    huffman::HuffmanEncoderDecoder d1_decoder(huffman::d1_freq);
    huffman::HuffmanEncoderDecoder data_size_decoder(huffman::data_size_freq);

    huffman::HuffmanEncoderDecoder literal_len_decoder(huffman::literal_lengths_freq);
    huffman::HuffmanEncoderDecoder match_len_decoder(huffman::match_len_freq);
    huffman::HuffmanEncoderDecoder cell_id_offset_decoder(huffman::cell_id_offset_freq);
    huffman::HuffmanEncoderDecoder offset_within_cell_decoder(huffman::offset_within_cell_freq);

    const unsigned n_cells = lz_bits.get_uint(BITS_FOR_N_CELLS);
    lz_bits.offs += BITS_FOR_N_CELLS;
    // std::cout << "decode: n_cells=" << n_cells << std::endl;
    // exit(0);

    std::vector<Ptr<MyCell> > my_cells(n_cells);
    for (unsigned i = 0; i < n_cells; ++i) {
        my_cells[i] = std::make_shared<MyCell>();
    }
    std::vector<CellMeta> cell_metas(n_cells);

    const auto store_out_uint = [&](const uint64_t x, const uint8_t blen) {
        CHECK(blen == 1);
        // CHECK(serialized_bits_encode[out_bits_write_ptr] == x);

        const auto initial_offs = out_bits.offs;
        out_bits.offs = out_bits_write_ptr;
        out_bits.store_uint(x, blen);
        out_bits_write_ptr += blen;
        out_bits.offs = initial_offs;
    };

    const auto try_read_d1 = [&](const unsigned i) -> bool {
        auto& my_cell = my_cells[i];
        const auto d1 = d1_decoder.decode_symbol(out_bits, out_bits_write_ptr);
        if (d1 == -1u) {
            return false;
        }
        // std::cout << "decode: d1=" << d1 << std::endl;
        my_cell->cnt_refs = d1 & 7;
        CHECK(my_cell->cnt_refs <= 4);
        my_cell->is_special = d1 & 8;

        // std::cout << "push d1 to cell with i=" << i << std::endl;
        // CHECK(my_cells_encode[i]->cnt_refs == my_cell->cnt_refs);
        // CHECK(my_cells_encode[i]->is_special == my_cell->is_special);
        return true;
    };

    const auto try_read_data_size = [&](const unsigned i) -> bool {
        const auto data_size = data_size_decoder.decode_symbol(out_bits, out_bits_write_ptr);
        if (data_size == -1u) {
            return false;
        }
        my_cells[i]->cnt_bits = data_size;
        return true;
    };

    const auto try_read_refs = [&](const unsigned i) -> bool {
        auto& my_cell = my_cells[i];
        const auto cnt_refs = my_cell->cnt_refs;
        CHECK(cnt_refs > 0);

        const auto initial_offs = out_bits.offs;
        std::array<unsigned, 4> refs{};
        const auto bit_len = len_in_bits(n_cells - 1);
        bool fail = false;
        for (unsigned j = 0; j < cnt_refs; j++) {
            if (out_bits.offs + bit_len > out_bits_write_ptr) {
                fail = true;
                break;
            }
            const unsigned id_store = out_bits.get_uint(bit_len) + 1;
            out_bits.offs += bit_len;
            unsigned ref_id = (n_cells + i - id_store) % n_cells;
            refs[j] = ref_id;
        }
        if (fail) {
            out_bits.offs = initial_offs;
            return false;
        }
        my_cell->refs = refs;
        return true;
    };

    const auto try_read_data = [&](const unsigned i) -> bool {
        auto& my_cell = my_cells[i];
        const auto cnt_bits = my_cell->cnt_bits;
        CHECK(cnt_bits > 0);
        if (out_bits.offs + cnt_bits > out_bits_write_ptr) {
            return false;
        }
        my_cell->data.resize((my_cell->cnt_bits + 7) / 8, 0);
        td::BitPtr my_cell_bit_ptr(my_cell->data.data(), 0);
        for (unsigned j = 0; j < my_cell->cnt_bits; ++j) {
            my_cell_bit_ptr.store_uint(out_bits.get_uint(1), 1);
            out_bits.offs += 1;
            my_cell_bit_ptr.offs += 1;
        }
        return true;
    };

    const auto next_meta_dummy = [&](const SerializeMetaData& m) {
        if (m.cell_id + 1 < n_cells) {
            return SerializeMetaData{m.cell_id + 1, m.field_type};
        }
        if (m.field_type == FieldType::data) {
            return SerializeMetaData{-1u, FieldType::none};
        }
        const auto fid = static_cast<size_t>(m.field_type);
        return SerializeMetaData{0u, static_cast<FieldType>(fid + 1)};
    };

    const auto predict_next_meta = [&](SerializeMetaData m) {
        if (m.cell_id == -1u) {
            return SerializeMetaData{0u, FieldType::d1};
        }
        while (true) {
            m = next_meta_dummy(m);
            if (m.field_type == FieldType::none) {
                return m;
            }
            if (m.field_type == FieldType::d1 || m.field_type == FieldType::data_size) {
                return m;
            }
            if (m.field_type == FieldType::refs) {
                if (my_cells[m.cell_id]->cnt_refs > 0) {
                    return m;
                }
            }
            if (m.field_type == FieldType::data) {
                if (my_cells[m.cell_id]->cnt_bits > 0) {
                    return m;
                }
            }
        }
        CHECK(false);
    };

    SerializeMetaData last_read_meta{-1u, FieldType::none};

    // populate meta up to out_bits_write_ptr (including potentially unreadable suffix)
    // also populate my_cells while we can
    // bits order: |all d1| |all data_size| |all refs| |all data|
    const auto advance_cells_read = [&]() {
        while (true) {
            const auto m = predict_next_meta(last_read_meta);
            const auto initial_offs = out_bits.offs;
            if (m.field_type == FieldType::none) {
                break;
            }
            if (m.field_type == FieldType::d1) {
                if (!try_read_d1(m.cell_id)) {
                    break;
                }
                cell_metas[m.cell_id].d1_start = initial_offs;
                cell_metas[m.cell_id].d1_cnt = out_bits.offs - initial_offs;
            } else if (m.field_type == FieldType::data_size) {
                if (!try_read_data_size(m.cell_id)) {
                    break;
                }
                cell_metas[m.cell_id].data_size_start = initial_offs;
                cell_metas[m.cell_id].data_size_cnt = out_bits.offs - initial_offs;
            } else if (m.field_type == FieldType::refs) {
                if (!try_read_refs(m.cell_id)) {
                    break;
                }
                cell_metas[m.cell_id].refs_start = initial_offs;
                cell_metas[m.cell_id].refs_cnt = out_bits.offs - initial_offs;
            } else if (m.field_type == FieldType::data) {
                if (!try_read_data(m.cell_id)) {
                    break;
                }
                cell_metas[m.cell_id].data_start = initial_offs;
                cell_metas[m.cell_id].data_cnt = out_bits.offs - initial_offs;
            } else {
                CHECK(false);
            }
            last_read_meta = m;
        }
        if (out_bits.offs < out_bits_write_ptr) {
            const auto m = predict_next_meta(last_read_meta);
            if (m.field_type == FieldType::d1) {
                cell_metas[m.cell_id].d1_start = out_bits.offs;
                cell_metas[m.cell_id].d1_cnt = out_bits_write_ptr - out_bits.offs;
            } else if (m.field_type == FieldType::data_size) {
                cell_metas[m.cell_id].data_size_start = out_bits.offs;
                cell_metas[m.cell_id].data_size_cnt = out_bits_write_ptr - out_bits.offs;
            } else if (m.field_type == FieldType::refs) {
                cell_metas[m.cell_id].refs_start = out_bits.offs;
                cell_metas[m.cell_id].refs_cnt = out_bits_write_ptr - out_bits.offs;
            } else if (m.field_type == FieldType::data) {
                cell_metas[m.cell_id].data_start = out_bits.offs;
                cell_metas[m.cell_id].data_cnt = out_bits_write_ptr - out_bits.offs;
            } else {
                CHECK(false);
            }
        }
    };

    // unsigned debug_ptr = 0;
    // unsigned literal_debug_ptr = 0;
    const auto advance_lz_decoder = [&]() {
        CHECK(lz_bits.offs < lz_cnt_bits);

        // read literal len
        auto literal_len = literal_len_decoder.decode_symbol(lz_bits);
        if (literal_len == LITERAL_LEN_TH) {
            literal_len += read_dumped_num(lz_bits, BITS_TO_DUMP_LITERAL_LEN);
        }
        // std::cout << "decode: literal_len=" << literal_len << std::endl;
        // CHECK(literal_len == literal_lengths_encode[debug_ptr]);

        // read literal
        for (unsigned j = 0; j < literal_len; ++j) {
            const bool bit = lz_bits.get_uint(1);
            // CHECK(bit == literal_bits_encode[literal_debug_ptr]);
            // literal_debug_ptr++;
            lz_bits.offs += 1;
            store_out_uint(bit, 1);
        }
        advance_cells_read();
        if (lz_bits.offs >= lz_cnt_bits) {
            CHECK(lz_bits.offs == lz_cnt_bits);
            return;
        }

        // read match len
        unsigned match_len = match_len_decoder.decode_symbol(lz_bits);
        if (match_len == MATCH_LEN_TH) {
            match_len += read_dumped_num(lz_bits, BITS_TO_DUMP_MATCH_LEN);
        }
        // std::cout << "decode: match_len=" << match_len << std::endl;
        // CHECK(match_len == match_lengths_encode[debug_ptr]);
        if (match_len == 0) {
            return;
        }

        // read offset of cell id
        unsigned cell_id_offset = cell_id_offset_decoder.decode_symbol(lz_bits);
        if (cell_id_offset == CELL_ID_OFFSET_TH) {
            const auto blen = len_in_bits((n_cells - 1) - CELL_ID_OFFSET_TH);
            cell_id_offset += lz_bits.get_uint(blen);
            lz_bits.offs += blen;
        }
        const auto cur_cell_id = predict_next_meta(last_read_meta).cell_id; //meta.at(out_bits_write_ptr).cell_id;
        const auto ref_cell_id = (n_cells + cur_cell_id - cell_id_offset) % n_cells;
        // std::cout << "decode: cell_id_offset=" << cell_id_offset << std::endl;
        // CHECK(cell_id_offset == cell_id_offset_encode[debug_ptr]);
        // std::cout << "decode: cur_cell_id=" << cur_cell_id << ", correct="
        //         << cur_cell_id_encode[debug_ptr].first
        //         << ", correct bit ptr=" << cur_cell_id_encode[debug_ptr].second << std::endl;
        // std::cout << "lz bits ptr = " << lz_bits.offs << std::endl;
        // CHECK(cur_cell_id == cur_cell_id_encode[debug_ptr].first);

        // read offset within ref_cell_id
        unsigned offset_within_ref_cell = offset_within_cell_decoder.decode_symbol(lz_bits);
        if (offset_within_ref_cell == OFFSET_WITHIN_CELL_TH) {
            offset_within_ref_cell += read_dumped_num(lz_bits, BITS_FOR_WITHIN_CELL_OFFSET_CURPLUS);
        }

        // std::cout << "decode: ref_cell_id=" << ref_cell_id << ", offset_within_ref_cell="
        //         << offset_within_ref_cell << std::endl;
        // CHECK(offset_within_ref_cell == within_cell_offset_encode[debug_ptr]);

        // std::cout << "decode: cell_offset_map[ref_cell_id].size()=" << cell_offset_map[ref_cell_id].size() << std::endl;
        const auto start = cell_metas.at(ref_cell_id).get_pos(offset_within_ref_cell); //cell_offset_map.at(ref_cell_id).at(offset_within_ref_cell);
        // debug
        // CHECK(start == match_start_encode[debug_ptr]);

        td::ConstBitPtr read_bit(out_bytes.data(), start);
        for (unsigned j = 0; j < match_len; ++j) {
            const bool bit = read_bit.get_uint(1);
            read_bit.offs += 1;
            store_out_uint(bit, 1);
        }
        advance_cells_read();

        // std::cout << "---" << std::endl;
    };

    while (lz_bits.offs < lz_cnt_bits) {
        advance_lz_decoder();
        // ++debug_ptr;
    }

    return my_cells_to_vm_root(my_cells);
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

    td::Ref<vm::Cell> root = deserialise(S);
    td::BufferSlice serialised_properly = vm::std_boc_serialize(root, 31).move_as_ok();
    return base64_encode(serialised_properly);
}
};
