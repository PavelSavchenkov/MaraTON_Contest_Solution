#include <string>

#include "serializator.hpp"
#include "utils.h"
#include "utils_local.hpp"

using namespace Serializator;

enum class Types {
    Block,
    BlockInfo
};

std::vector<Ptr<MyCell>> cells;

void dfs(const Ptr<MyCell>& cell, const Types type = Types::Block) {
    if (type == Types::Block) {
        // block#11ef55aa global_id:int32
        // info:^BlockInfo value_flow:^ValueFlow
        // state_update:^(MERKLE_UPDATE ShardState)
        // extra:^BlockExtra = Block;
        CHECK(cell->cnt_bits >= 4 * 8);
        td::ConstBitPtr data_ptr(cell->data.data(), 0);

        const uint32_t pref = data_ptr.get_uint(4 * 8);
        data_ptr.offs += 4 * 8;

        CHECK(pref == 0x11ef55aa);
        std::cout << "first 4 bytes: 0x" << std::hex << pref << std::dec << std::endl;

        uint32_t global_id = data_ptr.get_uint(32);
        data_ptr.offs += 32;

        std::cout << "global_id: " << global_id << std::endl;
        CHECK(4294967057 == global_id);

        CHECK(data_ptr.offs == cell->cnt_bits);

        dfs(cells[cell->refs[0]], Types::BlockInfo);
        return;
    }

    if (type == Types::BlockInfo) {

    }
}

void analyse(const std::string& block_path) {
    td::Ref<vm::Cell> root_vm = read_from_filepath_to_root_cell(block_path);
    cells = convert_to_my_cells(root_vm);

    const auto root = cells[0];

    dfs(root);
}

int main() {
    const std::string pref = "/Users/pavel/Programming/MaraTON/ton-sample-tests/";
    const std::string files_names_file = pref + "file_list.txt";

    std::ifstream files_names(files_names_file);
    std::string line;
    while (files_names >> line) {
        if (line.empty() || line[0] != '1') {
            continue;
        }
        line = pref + line;

        // line = "/Users/pavel/Programming/MaraTON/ton-sample-tests/1-025.txt";

        analyse(line);

        // break;
    }
}
