#include <iostream>
#include <fstream>

#include "huffman.hpp"
#include "utils.h"
#include "vm/excno.hpp"
#include "utils_local.hpp"

std::set<const vm::Cell *> was;
std::map<td::uint8, unsigned> cnt_type;
std::map<td::uint8, td::uint32> sum_data_sizes;
std::map<unsigned, unsigned> cnt_with_level;

std::map<const vm::Cell *, int> depth;

std::map<uint64_t, unsigned> cnt_data_bytes;

std::map<const vm::Cell *, std::set<const vm::Cell *> > parents;
std::map<unsigned, unsigned> cnt_refs_map;

std::vector<unsigned> cnt_d1;
std::vector<unsigned> cnt_data_size;
std::vector<unsigned> cnt_d1_d2;

void dfs(const td::Ref<vm::Cell> &cell, bool is_root = false) {
    if (was.count(cell.get())) {
        return;
    }
    was.insert(cell.get());
    depth[cell.get()] = 0;
    vm::Cell::LoadedCell cell_loaded = cell->load_cell().move_as_ok();

    // find out level of the cell
    ++cnt_with_level[cell_loaded.data_cell->get_level()];

    // find out cell type
    CellType cell_type = cell_loaded.data_cell->special_type();
    const auto cell_type_int = cell_type_to_int(cell_type);
    ++cnt_type[cell_type_int];

    // find out data size
    sum_data_sizes[cell_type_int] += cell_loaded.data_cell->size();

    const auto cnt_refs = cell_loaded.data_cell->get_refs_cnt();
    ++cnt_refs_map[cnt_refs];
    for (unsigned i = 0; i < cnt_refs; i++) {
        td::Ref<vm::Cell> ref = cell_loaded.data_cell->get_ref(i);
        parents[ref.get()].insert(cell.get());
        dfs(ref);

        CHECK(depth.count(ref.get()));
        depth[cell.get()] = std::max(depth[cell.get()], depth[ref.get()] + 1);
    }

    const uint8_t d1 = cnt_refs + 8 * (cell_loaded.data_cell->is_special());
    ++cnt_d1[d1];

    const unsigned data_size = cell_loaded.data_cell->size();
    ++cnt_data_size[data_size];

    ++cnt_d1_d2[(data_size << 4u) ^ d1];

    // if (is_root) {
    //     vm::CellSlice cell_slice = cell_to_slice(cell);
    //     uint64_t bytes = cell_slice.fetch_ulong(32);
    //     std::cout << std::hex << "root 4 bytes: 0x" << bytes << std::dec << std::endl;
    //     std::cout << "number of references: " << cell_loaded.data_cell->get_refs_cnt() << std::endl;
    //     std::cout << "cnt data bits: " << cell_loaded.data_cell->get_bits() << std::endl;
    //     exit(0);
    // }

    static uint8_t bits_to_look = 8;
    if (cell_type == vm::Cell::SpecialType::Ordinary && cell_loaded.data_cell->size() >= bits_to_look) {
        vm::CellSlice cell_slice = cell_to_slice(cell);
        uint64_t bytes = cell_slice.fetch_ulong(bits_to_look);
        cnt_data_bytes[bytes]++;
    }

    if (cell_type == vm::Cell::SpecialType::MerkleUpdate) {
        vm::CellSlice cell_slice = cell_to_slice(cell);

        while (cell_slice.size_refs() > 0) {
            td::Ref<vm::Cell> ref = cell_slice.fetch_ref();
        }

        std::cout << "first byte of merkle update data = " << std::bitset<8>(cell_slice.fetch_long(8)) << std::endl;

        // skip 256 bits of "from hash"
        std::vector<unsigned char> first_hash;
        for (int i = 0; i < 256 / 8; ++i) {
            first_hash.push_back(cell_slice.fetch_long(8));
        }

        // skip 256 bits of "to hash"
        cell_slice.fetch_bits(256);

        long from_depth = cell_slice.fetch_long(16);
        long to_depth = cell_slice.fetch_long(16);

        td::Ref<vm::Cell> ref = cell_loaded.data_cell->get_ref(0);

        std::cout << "from_depth = " << from_depth << std::endl;
        std::cout << "to_depth = " << to_depth << std::endl;
        std::cout << "cnt refs = " << cell_loaded.data_cell->get_refs_cnt() << std::endl;
        std::cout << "depth of \"merkle update\" cell calculated by me = " << depth[cell.get()] << std::endl;
        std::cout << "depth of the first ref = " << ref->get_depth(0) << std::endl;
        std::cout << "level of the \"merkle update\" cell = " << cell_loaded.data_cell->get_level() << std::endl;

        vm::CellSlice ref_slice = cell_to_slice(ref);
        long constructor_id = ref_slice.fetch_long(32);
        std::cout << "first 4 bytes in the first referenced cell = 0x" << std::hex << constructor_id << std::endl;

        const auto ref_hash = ref->get_hash(0);
        std::cout << "(my    ) first byte of hash of the first ref = " << std::bitset<8>(ref_hash.as_slice()[0]) <<
                std::endl;
        std::cout << "(stored) first byte of hash of the first ref = " << std::bitset<8>(first_hash[0]) << std::endl;

        std::cout << "levels of sons of \"merkle update\" cell" << std::endl;
        for (int i = 0; i < 2; ++i) {
            td::Ref<vm::Cell> son = cell_loaded.data_cell->get_ref(i);
            std::cout << son->get_level() << " ";
        }
        std::cout << std::endl;

        // cell_slice = cell_to_slice(cell);
        // block::gen::MERKLE_UPDATE::Record merkle_rec;
        // bool success = block::gen::MERKLE_UPDATE(block::gen::AccountState()).unpack(cell_slice, merkle_rec);
        // CHECK(success);
        // std::cout << merkle_rec.old.get() << " " << cell_loaded.data_cell->get_ref(0).get() << std::endl;

        std::cout << "-----------" << std::endl;
        std::cout << std::dec;
    }

    // ANOTHER WAY: traverse using Cell Slice
    {
        // vm::CellSlice cellSlice = cell_to_slice(cell);
        //
        // while (cellSlice.size_refs() > 0) {
        //     td::Ref<vm::Cell> ref = cellSlice.fetch_ref();
        //     dfs(ref);
        // }
    }
}

void analyse(const std::string &block_path) {
    td::Ref<vm::Cell> root = read_from_filepath_to_root_cell(block_path);

    was.clear();
    depth.clear();
    cnt_with_level.clear();

    try {
        dfs(root, true);
    } catch (vm::VmError &e) {
        std::cout << "Exception in dfs: " << e.get_msg() << std::endl;
    }

    std::cout << "visited: " << was.size() << std::endl;
    for (const auto &it: cnt_type) {
        std::cout << cell_type_int_to_string(it.first) << ": ";
        const auto cnt = it.second;
        std::cout << "cnt = " << cnt << ", avg data bits = " << sum_data_sizes[it.first] * 1.0 / cnt << std::endl;
    }
    std::cout << "Levels: " << std::endl;
    for (const auto &it: cnt_with_level) {
        std::cout << "level=" << it.first << ", its cnt=" << it.second << std::endl;
    }
    std::cout << "root depth = " << depth[root.get()] << std::endl;

    std::vector<std::pair<unsigned, uint64_t> > all_bytes;
    for (const auto &it: cnt_data_bytes) {
        uint64_t bytes = it.first;
        unsigned cnt = it.second;
        all_bytes.emplace_back(cnt, bytes);
    }
    std::sort(all_bytes.begin(), all_bytes.end());
    for (const auto &it: all_bytes) {
        uint64_t bytes = it.second;
        unsigned cnt = it.first;
        std::cout << std::hex << "0x" << std::setfill('0') << std::setw(2) << bytes << std::dec << " -> " << cnt <<
                std::endl;
    }

    std::map<unsigned, unsigned> cnt_parents;
    std::cout << "------------------\nparent stats: " << std::endl;
    for (const auto &it: parents) {
        const auto node = it.first;
        const auto pars = it.second;
        ++cnt_parents[pars.size()];
        // if (it.second.size() == 28) {
        //     vm::Cell::LoadedCell cell_loaded = it.first->load_cell().move_as_ok();
        //     const unsigned cnt_bits = it.first->load_cell().move_as_ok().data_cell->get_bits();
        //     auto cell_slice = cell_to_slice(td::Ref<vm::Cell>(it.first));
        //     std::cout << "node with 28 parents: " << std::endl;
        //     std::cout << "cnt bits: " << cnt_bits << std::endl;
        //     std::cout << "is special: " << it.first->load_cell().move_as_ok().data_cell->is_special() << std::endl;
        //     std::cout << "first 1 byte: 0x" << std::hex << cell_slice.fetch_ulong(8) << std::dec << std::endl;
        //     for (int i = 0; i < 3; ++i) cell_slice.fetch_ulong(8);
        //     for (int i = 0; i + 8 * 4 < cnt_bits; ++i) std::cout << cell_slice.fetch_ulong(1);
        //     std::cout << std::endl;
        // }
    }
    std::cout << "cnt nodes with the number of parents = x" << std::endl;
    for (const auto &it: cnt_parents) {
        std::cout << "cnt parents = " << it.first << ", nodes with such cnt = " << it.second << std::endl;
    }
    std::cout << std::endl;
    std::cout << "----------------\ncnt nodes with the number of refs = x" << std::endl;
    unsigned sum = 0;
    for (const auto &it: cnt_refs_map) {
        sum += it.second;
    }
    const std::vector<unsigned> lens{2, 3, 1, 4, 4};
    double sum_bits = 0;
    double sum_prob = 0;
    for (const auto &it: cnt_refs_map) {
        const double prob = it.second * 1.0 / sum;
        std::cout << "cnt refs = " << it.first << ", nodes with such cnt = " << it.second
                << ", prob = " << prob << std::endl;
        sum_bits += prob * lens[it.first];
        sum_prob += prob;
    }
    std::cout << "avg_bits = " << sum_bits << ", sum_prob = " << sum_prob << std::endl;
    std::cout << std::endl;
}

int main() {
    const std::string pref = "/Users/pavel/Programming/MaraTON/ton-sample-tests/";
    const std::string files_names_file = pref + "file_list.txt";

    cnt_d1.assign(16, 0);
    cnt_data_size.assign(1024, 0);
    cnt_d1_d2.assign(1u << (10 + 4), 0);

    std::ifstream files_names(files_names_file);
    std::string line;
    while (files_names >> line) {
        if (line.empty() || line[0] != '1') {
            continue;
        }
        line = pref + line;

        // line = "/Users/pavel/Programming/MaraTON/ton-sample-tests/1-025.txt";

        analyse(line);
    }

    // d1
    {
        std::cout << "d1 stats: " << std::endl;
        std::cout << "static const std::vector<unsigned> d1_freq = {";
        for (const auto &it: cnt_d1) {
            std::cout << it << ",";
        }
        std::cout << "};" << std::endl;
        huffman::HuffmanEncoderDecoder tmp(cnt_d1, true);
    }

    // data sizes
    {
        std::cout << "data sizes stats: " << std::endl;
        std::cout << "static const std::vector<unsigned> data_size_freq = {";
        for (const auto &it: cnt_data_size) {
            std::cout << it << ",";
        }
        std::cout << "};" << std::endl;
        huffman::HuffmanEncoderDecoder tmp(cnt_data_size, true);
    }

    // both
    {
        std::cout << "d1_d2 stats: " << std::endl;
        std::cout << "static const std::vector<unsigned> d1_d2_freq = {";
        for (const auto &it: cnt_d1_d2) {
            std::cout << it << ",";
        }
        std::cout << "};" << std::endl;
        huffman::HuffmanEncoderDecoder tmp(cnt_d1_d2, true);
    }
}
