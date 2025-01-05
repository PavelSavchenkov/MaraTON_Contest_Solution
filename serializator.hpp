#pragma once

#include "utils.h"
#include "compressor.hpp"
#include "huffman.hpp"

namespace Serializator {

const std::string compressed_dict = "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";
std::basic_string<uint8_t> uncompress_dict() {
    // return {};

    td::Slice slice(compressed_dict.data(), compressed_dict.size());
    std::string dict_bytes_str = td::base64_decode(slice).move_as_ok();
    std::basic_string<uint8_t> dict_bytes(reinterpret_cast<uint8_t*>(dict_bytes_str.data()), dict_bytes_str.size());
    const auto final_dict = LZ_compressor::decompress(dict_bytes);
    // std::cout << "final_dict.size() = " << final_dict.size() << std::endl;
    return final_dict;
}
const std::basic_string<uint8_t> uncompressed_dict = uncompress_dict();

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
        const auto type = static_cast<CellType>(data.at(0));
        CHECK(type != CellType::Ordinary);
        return type;
    }

    MyCell() {}

    explicit MyCell(td::Ref<vm::Cell> cell) {
        vm::Cell::LoadedCell cell_loaded = cell->load_cell().move_as_ok();
        CellType cell_type = cell_loaded.data_cell->special_type();
        is_special = (cell_type != CellType::Ordinary);
        cnt_refs = cell_loaded.data_cell->get_refs_cnt();

        cnt_bits = cell_loaded.data_cell->size();

        vm::CellSlice cell_slice = cell_to_slice(cell);
        unsigned bits_left = cnt_bits;
        while (bits_left >= 8) {
            uint8_t byte = cell_slice.fetch_ulong(8);
            data.push_back(byte);
            bits_left -= 8;
        }
        if (bits_left > 0) {
            CHECK(bits_left < 8);
            uint8_t last_byte = cell_slice.fetch_ulong(bits_left);
            last_byte <<= 8 - bits_left;
            data.push_back(last_byte);

            CHECK(((last_byte >> (7 - bits_left)) & 1) == 0);
            // last_byte ^= 1u << (7 - bits_left);
        } else {
            CHECK(bits_left == 0);
        }

        const unsigned cnt_refs = cell_loaded.data_cell->get_refs_cnt();
        CHECK(cnt_refs <= 4);
        for (unsigned i = 0; i < cnt_refs; i++) {
            td::Ref<vm::Cell> ref = cell_loaded.data_cell->get_ref(i);
            refs_as_cells[i] = ref;
        }

        if (cell_type == CellType::MerkleUpdate || cell_type == CellType::MerkleProof) {
            data.resize(1);
            cnt_bits = 8;
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

    bool operator==(const MyCell &rhs) const {
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

    void print() const {
        std::cout << "MyCell|cnt_bits=" << cnt_bits;
        std::cout << ", is_special=" << is_special << ", cnt_refs=" << cnt_refs;
        std::cout << ", data=";
        for (const auto &c: data) {
            std::cout << std::bitset<8>(c) << " ";
        }
        std::cout << std::dec;
        std::cout << ", refs=";
        for (const auto &r: refs) {
            std::cout << r << " ";
        }
        std::cout << "|" << std::endl;
    }
};

std::vector<Ptr<MyCell> > convert_to_my_cells(td::Ref<vm::Cell> root) {
    std::vector<Ptr<MyCell> > my_cells;

    std::unordered_map<const vm::Cell *, unsigned> pos;
    auto dfs = [&](auto &&self, td::Ref<vm::Cell> cell) -> void {
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

    for (const auto &cell: my_cells) {
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

        // sort inside buckets with the same byte length
        auto cmp_desc = [&](unsigned i, unsigned j) {
            const auto d1_i = my_cells[i]->d1();
            const auto d1_j = my_cells[j]->d1();
            if (d1_i != d1_j) {
                return d1_i < d1_j;
            }

            const auto d2_i = my_cells[i]->d2();
            const auto d2_j = my_cells[j]->d2();
            if (d2_i != d2_j) {
                return d2_i < d2_j;
            }


            if (my_cells[i]->data != my_cells[j]->data) {
                return my_cells[i]->data < my_cells[j]->data;
            }

            if (my_cells[i]->refs != my_cells[j]->refs) {
                return my_cells[i]->refs < my_cells[j]->refs;
            }

            return i < j;
        };

        uint8_t bytes = 1;
        for (unsigned l = 0, r = 0; l < n; l = r, ++bytes) {
            r = std::min(1u << (8 * bytes), n);

            // [l, r)
            std::sort(std::begin(order) + l, std::begin(order) + r, cmp_desc);
        }

        // update ref indexes
        std::vector<unsigned> pos(n);
        for (unsigned i = 0; i < n; i++) {
            pos[order[i]] = i;
        }

        for (auto &my_cell: my_cells) {
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

    // the number of bytes needed to store a cell index and its references
    uint8_t bytes_for_index = 1;

    std::basic_string<uint8_t> bytes_d1;
    uint8_t byte_d1 = 0xFF;
    std::basic_string<uint8_t> bytes_d2;
    std::basic_string<uint8_t> bytes_refs;
    std::basic_string<uint8_t> bytes_data;

    for (unsigned i = 0; i < n; ++i) {
        const auto my_cell = my_cells[i];
        if (i == (1u << (8 * bytes_for_index))) {
            ++bytes_for_index;
        }

        // serialise d1 into 4 bits
        {
            const auto d1 = my_cell->d1();
            CHECK((d1 & ~0x0F) == 0);
            if (i % 2 == 0) {
                // start new byte
                byte_d1 = d1 << 4;
            } else {
                CHECK((byte_d1 & 0x0F) == 0);
                byte_d1 ^= d1;
                bytes_d1.push_back(byte_d1);
                byte_d1 = 0xFF;
            }
        }

        bytes_d2.push_back(my_cell->d2());
        for (unsigned it = 0; it < my_cell->cnt_bits / 8; ++it) {
            bytes_data.push_back(my_cell->data[it]);
        }
        if (my_cell->cnt_bits % 8 != 0) {
            uint8_t pos = 7 - (my_cell->cnt_bits % 8);
            uint8_t last_byte = my_cell->data.at((my_cell->cnt_bits + 7) / 8 - 1);
            last_byte ^= (1u << pos);
            bytes_data.push_back(last_byte);
        }

        // up to 4 refs
        for (unsigned it = 0; it < my_cell->cnt_refs; ++it) {
            // big endian
            push_as_bytes(bytes_refs, my_cell->refs[it], bytes_for_index);
        }
    }
    if (byte_d1 != 0xFF) {
        // fill first 4 bits with non-existent descriptor
        CHECK((byte_d1 & 0x0F) == 0);
        byte_d1 |= 0x0F;
        bytes_d1.push_back(byte_d1);
    }

    std::basic_string<uint8_t> bytes;
    bytes += bytes_d1;
    bytes += bytes_refs;
    bytes += bytes_data;
    std::reverse(std::begin(bytes_d2), std::end(bytes_d2));
    bytes += bytes_d2;

    return bytes;
}

td::BufferSlice serialize_slice(td::Ref<vm::Cell> root) {
    auto bytes = serialize(root);
    return td::BufferSlice(reinterpret_cast<const char *>(bytes.data()), bytes.size());
}

td::Ref<vm::Cell> deserialise(std::basic_string<uint8_t> buffer) {
    std::vector<Ptr<MyCell> > my_cells;

    // build my_cells
    uint8_t bytes_for_index = 1;
    unsigned sum_bytes_refs = 0;
    unsigned sum_bytes_data = 0;
    for (unsigned i = 0; i / 2 < buffer.size(); ++i) {
        Ptr<MyCell> my_cell = std::make_shared<MyCell>();

        // update the number of bytes needed to store outgoing refs
        {
            unsigned cur_cell_pos = i + 1;
            if (cur_cell_pos == (1u << (8 * bytes_for_index))) {
                ++bytes_for_index;
            }
        }

        // read first descriptor byte
        {
            uint8_t desc = (i % 2 == 0) ? (buffer[i / 2] >> 4) : (buffer[i / 2] & 0x0F);
            CHECK((desc & 0xF0) == 0);
            unsigned r = desc & 7;
            CHECK(r <= 4);
            bool s = (desc >> 3) & 1;

            my_cell->cnt_refs = r;
            my_cell->is_special = s;

            sum_bytes_refs += r * bytes_for_index;
        }

        // second descriptor byte
        {
            uint8_t desc = buffer[buffer.size() - i - 1];
            size_t cnt_bytes = (desc >> 1) + (desc & 1);
            sum_bytes_data += cnt_bytes;
            my_cell->cnt_bits = desc; // bad practice
        }

        my_cells.push_back(my_cell);

        const unsigned cnt_bytes_pref = (i / 2) + 1;
        const unsigned cnt_bytes_suff = i + 1;
        const unsigned sum_bytes = cnt_bytes_pref + sum_bytes_refs + sum_bytes_data + cnt_bytes_suff;
        if (sum_bytes >= buffer.size()) {
            CHECK(sum_bytes == buffer.size());
            break;
        }
    }
    const auto n = my_cells.size();
    CHECK(!my_cells.empty());

    // read refs and data
    bytes_for_index = 1;
    unsigned ptr_refs = (n + 1) / 2;
    unsigned ptr_data = ptr_refs + sum_bytes_refs;
    for (unsigned i = 0; i < n; ++i) {
        auto &my_cell = my_cells[i];

        // update the number of bytes needed to store outgoing refs
        {
            unsigned cur_cell_pos = i + 1;
            if (cur_cell_pos == (1u << (8 * bytes_for_index))) {
                ++bytes_for_index;
            }
        }

        // read up to 4 refs
        {
            for (unsigned j = 0; j < my_cell->cnt_refs; j++) {
                // bytes_for_index bytes in big endian
                unsigned ref_id = 0;
                for (unsigned iter2 = 0; iter2 < bytes_for_index; ++iter2) {
                    ref_id = (ref_id << 8) ^ buffer[ptr_refs++];
                }
                my_cell->refs[j] = ref_id;
            }
        }

        // read data
        {
            const auto desc = my_cell->cnt_bits;
            my_cell->cnt_bits = 0;
            unsigned cnt_bytes = desc >> 1;
            while (cnt_bytes > 0) {
                my_cell->data.push_back(buffer[ptr_data++]);
                --cnt_bytes;
            }
            my_cell->cnt_bits = my_cell->data.size() * 8;
            if (desc & 1) {
                uint8_t tail = buffer[ptr_data++];
                CHECK(tail != 0);
                int cnt_trailing_zeros = __builtin_ctz(tail);
                CHECK(cnt_trailing_zeros <= 6);
                unsigned cnt_bits_pref = 7 - cnt_trailing_zeros;
                tail ^= 1 << cnt_trailing_zeros;

                my_cell->data.push_back(tail);
                my_cell->cnt_bits += cnt_bits_pref;
            }
        }
    }

    std::vector<unsigned> order;
    // top sort
    {
        std::vector<char> was(n, false);
        auto dfs = [&](auto &&self, unsigned i) -> void {
            CHECK(!was[i]);
            was[i] = true;

            for (unsigned j = 0; j < my_cells[i]->cnt_refs; j++) {
                unsigned to = my_cells[i]->refs[j];
                if (!was[to]) {
                    self(self, to);
                }
            }

            order.push_back(i);
        };
        for (unsigned i = 0; i < n; ++i) {
            if (!was[i]) {
                dfs(dfs, i);
            }
        }
    }

    uint8_t bytes_for_index_max = len_in_bytes(my_cells.size());

    // build DAG of td::Ref<vm::Cell>
    std::vector<td::Ref<vm::Cell> > cells(n);
    for (unsigned i: order) {
        const auto &my_cell = my_cells.at(i);

        vm::CellBuilder cell_builder;

        // add refs
        std::vector<td::Ref<vm::Cell> > refs;
        for (unsigned j = 0; j < my_cell->cnt_refs; ++j) {
            unsigned ref_id = my_cell->refs[j];
            CHECK(len_in_bytes(ref_id) <= len_in_bytes(i));
            CHECK(cells[ref_id].get());
            cell_builder.store_ref(cells[ref_id]);
            refs.push_back(cells[ref_id]);
        }

        const auto cell_type = my_cell->get_type();
        std::basic_string<uint8_t> data = my_cell->data;;
        unsigned cnt_bits = my_cell->cnt_bits;
        if (cell_type == CellType::MerkleUpdate || cell_type == CellType::MerkleProof) {
            // hash0 (256), hash1, depth0 (8 * bytes_for_index), depth1
            CHECK(refs.size() <= 2);
            CHECK(data.size() == 1);

            for (unsigned j = 0; j < refs.size(); ++j) {
                const auto hash = refs[j]->get_hash(0);
                const auto slice = hash.as_slice();
                auto calc_hash = std::basic_string<uint8_t>(slice.data(), slice.data() + (256 / 8));
                data += calc_hash;
            }

            for (unsigned j = 0; j < refs.size(); ++j) {
                unsigned depth = refs[j]->get_depth(0);
                push_as_bytes(data, depth, bytes_for_index_max);
            }
            cnt_bits = data.size() * 8;
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

        cells[i] = cell;
    }

    return cells[order.back()];
}

td::Ref<vm::Cell> deserialise_slice(td::Slice buffer_slice) {
    std::basic_string<uint8_t> buffer(buffer_slice.data(), buffer_slice.data() + buffer_slice.size());
    return deserialise(buffer);
}

std::string compress(
    const std::string &base64_data,
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
    S = LZ_compressor::compress(S, uncompressed_dict);
    // S = LZ_compressor::compress_standard(S);

    if (return_before_huffman) {
        return to_string(S);
    }
    S = huffman::encode_8(S);
    auto base64 = td::base64_encode(td::Slice(S.data(), S.size()));
    return base64;
}

std::string decompress(const std::string &base64_data) {
    CHECK(!base64_data.empty());
    std::string data = td::base64_decode(base64_data).move_as_ok();
    std::basic_string<uint8_t> S(data.begin(), data.end());
    S = huffman::decode_8(S);
    S = LZ_compressor::decompress(S, uncompressed_dict);
    // S = LZ_compressor::decompress_standard(S);
    td::Ref<vm::Cell> root = deserialise(S);
    td::BufferSlice serialised_properly = vm::std_boc_serialize(root, 31).move_as_ok();
    return base64_encode(serialised_properly);
}
};
