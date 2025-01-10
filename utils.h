#pragma once

#include <fstream>

#include "vm/cells/CellSlice.h"
#include "td/utils/lz4.h"
#include "td/utils/base64.h"
#include "vm/boc.h"

using CellType = vm::CellTraits::SpecialType;
template<typename T>
using Ptr = std::shared_ptr<T>;

std::string to_string(const std::basic_string<uint8_t> &bytes) {
    return {reinterpret_cast<const char *>(bytes.data()), bytes.size()};
}

td::uint8 cell_type_to_int(const CellType &type) {
    return static_cast<td::uint8>(type);
}

std::string cell_type_int_to_string(const td::uint8 &type_int) {
    static const std::string type_names[] = {"Ordinary", "PrunnedBranch", "Library", "MerkleProof", "MerkleUpdate"};
    CHECK(type_int < std::size(type_names));
    return type_names[type_int];
}

vm::CellSlice cell_to_slice(const td::Ref<vm::Cell> &cell) {
    vm::Cell::LoadedCell cell_loaded = cell->load_cell().move_as_ok();

    if (!cell_loaded.data_cell->is_special()) {
        return vm::load_cell_slice(cell);
    }

    bool is_special_flag;
    vm::CellSlice cell_slice = vm::load_cell_slice_special(cell, is_special_flag);
    CHECK(is_special_flag);
    return cell_slice;
}

td::Ref<vm::Cell> read_from_filepath_to_root_cell(const std::string &block_path) {
    std::ifstream in(block_path);

    // mode
    std::string tmp;
    in >> tmp;

    // base64 data
    std::string base64_data;
    in >> base64_data;
    CHECK(!base64_data.empty());

    td::BufferSlice binary_data(td::base64_decode(base64_data).move_as_ok());

    // uint32_t first_32bits = 0;
    // for (int i = 0; i < 4; ++i) {
    //     first_32bits = (first_32bits << 8) ^ uint8_t(binary_data[i]);
    // }
    // std::cout << "first 32 bits in hex: 0x" << std::hex << first_32bits << std::dec << std::endl;
    // exit(0);

    td::Ref<vm::Cell> root = vm::std_boc_deserialize(binary_data).move_as_ok();

    return root;
}

inline uint8_t len_in_bits(const uint64_t num) {
    const auto zeros = td::count_leading_zeroes64(num);
    return std::max(1, 64 - zeros);
}

inline uint8_t len_in_bytes(const uint64_t num) {
    const auto bits = len_in_bits(num);
    return (bits + 7) / 8;
}

inline void push_as_bytes(std::basic_string<uint8_t> &data, uint64_t number, uint8_t bytes) {
    CHECK(bytes > 0);
    std::basic_string<uint8_t> tmp;
    for (uint8_t i = 0; i < bytes; i++) {
        tmp.push_back(number & 0xFF);
        number >>= 8;
    }
    CHECK(number == 0);
    std::reverse(tmp.begin(), tmp.end());
    data += tmp;
}

inline void insert_bits_to_beginning(
    std::basic_string<uint8_t> &data,
    unsigned &data_cnt_bits,
    unsigned num,
    unsigned num_bits
) {
    const unsigned new_cnt_bits = data_cnt_bits + num_bits;
    std::basic_string<uint8_t> new_data((new_cnt_bits + 7) / 8, 0);
    td::BitPtr new_data_bits(new_data.data(), 0);

    new_data_bits.store_uint(num, num_bits);
    new_data_bits.offs += num_bits;

    td::BitPtr data_bits(data.data(), 0);

    while (data_cnt_bits > 0) {
        const unsigned cur_bits = std::min(data_cnt_bits, 32u);
        new_data_bits.store_uint(data_bits.get_uint(cur_bits), cur_bits);
        new_data_bits.offs += cur_bits;
        data_bits.offs += cur_bits;
        data_cnt_bits -= cur_bits;
    }

    data = new_data;
    data_cnt_bits = new_cnt_bits;
}

inline void push_bit_range(
    td::BitPtr &data,
    td::BitPtr bit_ptr,
    unsigned from,
    unsigned cnt
) {
    bit_ptr.offs = from;
    while (cnt > 0) {
        unsigned cur_cnt = std::min(cnt, 64u);
        data.store_uint(bit_ptr.get_uint(cur_cnt), cur_cnt);
        data.offs += cur_cnt;
        bit_ptr.offs += cur_cnt;
        cnt -= cur_cnt;
    }
}

inline void push_bit_range(
    std::basic_string<uint8_t> &data,
    unsigned &data_cnt_bits,
    td::BitPtr bit_ptr,
    unsigned from,
    unsigned cnt
) {
    td::BitPtr data_bit_ptr(data.data(), data_cnt_bits);
    push_bit_range(data_bit_ptr, bit_ptr, from, cnt);
    data_cnt_bits = data_bit_ptr.offs;
}

unsigned cnt_occurrences(const std::basic_string<uint8_t> &sample, const std::basic_string<uint8_t> &text) {
    const auto s = sample + text;
    const unsigned n = s.size();
    std::vector<unsigned> z(n);
    unsigned l = 0;
    unsigned r = 0;
    unsigned cnt = 0;
    for (unsigned i = 1; i < n; i++) {
        if (i < r) {
            z[i] = std::min(r - i, z[i - l]);
        }
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) {
            ++z[i];
        }
        if (i + z[i] > r) {
            l = i;
            r = i + z[i];
        }
        if (i >= sample.size() && z[i] >= sample.size()) {
            ++cnt;
        }
    }
    return cnt;
}

struct Timer {
    double start_time{};
    std::string name;

    explicit Timer(const std::string &name) : name(name) {
        start_time = clock() * 1.0 / CLOCKS_PER_SEC;
    }

    ~Timer() {
        // std::cout << name << ", Time: " << ((clock() * 1.0 / CLOCKS_PER_SEC) - start_time) << std::endl;
    }
};

std::basic_string<uint8_t> compress_lz_standard(const std::basic_string<uint8_t> &data) {
    auto res = td::lz4_compress(td::BufferSlice(reinterpret_cast<const char *>(data.data()), data.size()));
    return {reinterpret_cast<const uint8_t *>(res.data()), res.size()};
}

std::basic_string<uint8_t> decompress_lz_standard(const std::basic_string<uint8_t> &data) {
    auto res = td::lz4_decompress(
        td::BufferSlice(reinterpret_cast<const char *>(data.data()), data.size()),
        2 << 20
    ).move_as_ok();
    return {reinterpret_cast<const uint8_t *>(res.data()), res.size()};
}

// 1 in the highest bit which is not used
inline unsigned get_min_bit_str_not_in(const std::basic_string<uint8_t> &bits) {
    CHECK(!bits.empty());
    std::vector<char> was(bits.size() * 2, 0);
    for (uint8_t len = 1; len <= bits.size(); ++len) {
        std::fill(std::begin(was), was.begin() + (1u << len), 0);
        unsigned cur = 0;
        const unsigned cover_mask = (1u << len) - 1;
        for (unsigned i = 0; i < len; ++i) {
            cur = cur * 2 + bits[i];
        }
        was[cur] = 1;
        for (unsigned i = len; i < bits.size(); ++i) {
            cur = (cur * 2 + bits[i]) & cover_mask;
            was[cur] = 1;
        }
        for (unsigned mask = 0; mask < (1u << len); ++mask) {
            if (!was[mask]) {
                return (1u << len) ^ mask;
            }
        }
    }
    CHECK(false);
    return -1u;
}
