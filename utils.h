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

uint8_t len_in_bits(const uint64_t num) {
    const auto zeros = td::count_leading_zeroes64(num);
    return std::max(1, 64 - zeros);
}

uint8_t len_in_bytes(const uint64_t num) {
    const auto bits = len_in_bits(num);
    return (bits + 7) / 8;
}

void push_as_bytes(std::basic_string<uint8_t> &data, uint64_t number, uint8_t bytes) {
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
