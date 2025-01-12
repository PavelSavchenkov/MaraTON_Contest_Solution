#pragma once

#include <fstream>

#include "vm/cells/CellSlice.h"
#include "td/utils/lz4.h"
#include "td/utils/base64.h"
#include "vm/boc.h"

using CellType = vm::CellTraits::SpecialType;
template<typename T>
using Ptr = std::shared_ptr<T>;

vm::CellSlice cell_to_slice(const td::Ref<vm::Cell>& cell) {
    vm::Cell::LoadedCell cell_loaded = cell->load_cell().move_as_ok();

    if (!cell_loaded.data_cell->is_special()) {
        return vm::load_cell_slice(cell);
    }

    bool is_special_flag;
    vm::CellSlice cell_slice = vm::load_cell_slice_special(cell, is_special_flag);
    CHECK(is_special_flag);
    return cell_slice;
}

inline uint8_t len_in_bits(const uint64_t num) {
    const auto zeros = td::count_leading_zeroes64(num);
    return std::max(1, 64 - zeros);
}

inline uint8_t len_in_bytes(const uint64_t num) {
    const auto bits = len_in_bits(num);
    return (bits + 7) / 8;
}

inline void insert_bits_to_beginning(
    std::basic_string<uint8_t>& data,
    unsigned& data_cnt_bits,
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
    td::BitPtr& data,
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
    std::basic_string<uint8_t>& data,
    unsigned& data_cnt_bits,
    td::BitPtr bit_ptr,
    unsigned from,
    unsigned cnt
) {
    td::BitPtr data_bit_ptr(data.data(), data_cnt_bits);
    push_bit_range(data_bit_ptr, bit_ptr, from, cnt);
    data_cnt_bits = data_bit_ptr.offs;
}

template<typename T>
std::basic_string<uint8_t> bytes_str_to_bit_str(
    const T& bytes,
    const bool marked_end = false
) {
    unsigned cnt_bits = bytes.size() * 8;
    if (marked_end) {
        const auto byte = bytes.back();
        CHECK(byte != 0);
        const auto trailing = td::count_trailing_zeroes64(byte);
        CHECK(trailing <= 7);
        cnt_bits -= trailing + 1;
    }
    std::basic_string<uint8_t> bits(cnt_bits, 0);
    for (unsigned i = 0; i < bits.size(); ++i) {
        const auto byte = static_cast<uint8_t>(bytes[i / 8]);
        bits[i] = ((byte >> (7 - i % 8)) & 1);
    }
    return bits;
}

struct Timer {
    double start_time{};
    std::string name;

    explicit Timer(const std::string& name) : name(name) {
        start_time = clock() * 1.0 / CLOCKS_PER_SEC;
    }

    ~Timer() {
        // std::cout << name << ", Time: " << ((clock() * 1.0 / CLOCKS_PER_SEC) - start_time) << std::endl;
    }
};

std::string to_string(const std::basic_string<uint8_t>& bytes) {
    return std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size());

    std::stringstream res;
    for (const auto byte: bytes) {
        res << std::bitset<8>(byte);
    }
    return res.str();
}

std::basic_string<uint8_t> decompress_lz_standard(const std::basic_string<uint8_t>& data) {
    auto res = td::lz4_decompress(
        td::BufferSlice(reinterpret_cast<const char *>(data.data()), data.size()),
        2 << 20
    ).move_as_ok();
    return {reinterpret_cast<const uint8_t *>(res.data()), res.size()};
}

std::vector<unsigned> decode_vector(const std::string& base64) {
    std::string data = td::base64_decode(base64).move_as_ok();
    std::basic_string<uint8_t> S(data.begin(), data.end());

    S = decompress_lz_standard(S);
    td::ConstBitPtr bit_ptr(S.data(), 0);

    const unsigned n = bit_ptr.get_uint(4 * 8);
    bit_ptr.offs += 4 * 8;

    const auto bits = bit_ptr.get_uint(5);
    bit_ptr.offs += 5;

    std::vector<unsigned> res;
    for (unsigned i = 0; i < n; ++i) {
        auto num = bit_ptr.get_uint(bits);
        bit_ptr.offs += bits;
        res.push_back(num);
    }

    return res;
}
