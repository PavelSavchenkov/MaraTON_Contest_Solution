#include <iostream>
#include <fstream>

#include "utils.h"
#include "vm/excno.hpp"

std::set<const vm::Cell*> was;
std::map<td::uint8, unsigned> cnt_type;
std::map<td::uint8, td::uint32> sum_data_sizes;
std::map<unsigned, unsigned> cnt_with_level;

std::map<const vm::Cell*, int> depth;

void dfs(const td::Ref<vm::Cell>& cell) {
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

    for (unsigned i = 0; i < cell_loaded.data_cell->get_refs_cnt(); i++) {
        td::Ref<vm::Cell> ref = cell_loaded.data_cell->get_ref(i);
        dfs(ref);

        CHECK(depth.count(ref.get()));
        depth[cell.get()] = std::max(depth[cell.get()], depth[ref.get()] + 1);
    }

    if (cell_type == vm::Cell::SpecialType::MerkleUpdate) {
        vm::CellSlice cell_slice = cell_to_slice(cell);

        while (cell_slice.size_refs() > 0) {
            td::Ref<vm::Cell> ref = cell_slice.fetch_ref();
        }

        std::cout << "first byte of merkle update data = " << std::bitset<8>(cell_slice.fetch_long(8)) << std::endl;

        // skip 256 bits of "from hash"
        std::vector<unsigned char> first_hash;
        for (int i = 0; i < 256/8; ++i) {
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
        std::cout << "(my    ) first byte of hash of the first ref = " << std::bitset<8>(ref_hash.as_slice()[0]) << std::endl;
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

void analyse(const std::string& block_path) {
    td::Ref<vm::Cell> root = read_from_filepath_to_root_cell(block_path);

    was.clear();
    depth.clear();
    cnt_with_level.clear();

    try {
        dfs(root);
    } catch (vm::VmError& e) {
        std::cout << "Exception in dfs: " << e.get_msg() << std::endl;
    }

    std::cout << "visited: " << was.size() << std::endl;
    for (const auto& it : cnt_type) {
        std::cout << cell_type_int_to_string(it.first) << ": ";
        const auto cnt = it.second;
        std::cout << "cnt = " << cnt << ", avg data bits = " << sum_data_sizes[it.first] * 1.0 / cnt << std::endl;
    }
    std::cout << "Levels: " << std::endl;
    for (const auto& it : cnt_with_level) {
        std::cout << "level=" << it.first << ", its cnt=" << it.second << std::endl;
    }
    std::cout << "root depth = " << depth[root.get()] << std::endl;
}

int main() {
    static const std::string block_path = "/Users/pavel/Programming/MaraTON/ton-sample-tests/1-013.txt";

    analyse(block_path);
}
