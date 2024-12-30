/*
* solution.cpp
 *
 * Example solution.
 * This is (almost) how blocks are actually compressed in TON.
 * Normally, blocks are stored using vm::std_boc_serialize with mode=31.
 * Compression algorithm takes a block, converts it to mode=2 (which has less extra information) and compresses it using lz4.
 */
#include <iostream>

#include "serializator.h"
#include "td/utils/lz4.h"
#include "td/utils/base64.h"
#include "vm/boc.h"

td::BufferSlice compress(td::Slice data) {
    td::Ref<vm::Cell> root = vm::std_boc_deserialize(data).move_as_ok();

    // td::Result<vm::Cell::LoadedCell> loadedRootResult = root->load_cell();
    // vm::Cell::LoadedCell loadedRoot = loadedRootResult.move_as_ok();
    // std::cout << "Refs Cnt = " << loadedRoot.data_cell->get_refs_cnt() << std::endl;

    td::BufferSlice serialized = vm::std_boc_serialize(root, 2).move_as_ok();
    return td::lz4_compress(serialized);
}

td::BufferSlice decompress(td::Slice data) {
    td::BufferSlice serialized = td::lz4_decompress(data, 2 << 20).move_as_ok();
    auto root = vm::std_boc_deserialize(serialized).move_as_ok();
    return vm::std_boc_serialize(root, 31).move_as_ok();
}

int main() {
    std::string mode;
    std::cin >> mode;
    CHECK(mode == "compress" || mode == "decompress");

    std::string base64_data;
    std::cin >> base64_data;
    CHECK(!base64_data.empty());

    // {
    //     auto compressed = Serializator::compress(base64_data);
    //     auto decompressed = Serializator::decompress(compressed);
    //     std::cout << "initial base 64: " << base64_data.size() << std::endl;
    //     std::cout << "compressed base 64: " << compressed.size() << std::endl;
    //     std::cout << "decompressed base 64: " << decompressed.size() << std::endl;
    //     std::cout << "initial == final: " << (base64_data == decompressed) << std::endl;
    //     return 0;
    // }

    // td::BufferSlice data(td::base64_decode(base64_data).move_as_ok());

    if (mode == "compress") {
        // data = compress(data);
        std::cout << Serializator::compress(base64_data) << std::endl;
    } else {
        // data = decompress(data);
        std::cout << Serializator::decompress(base64_data) << std::endl;
    }
    return 0;

    // std::cout << td::base64_encode(data) << std::endl;
}
