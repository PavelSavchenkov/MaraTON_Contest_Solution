#include <iostream>
#include <bitset>
#include <fstream>

#include "vm/cells/CellSlice.h"
#include "td/utils/lz4.h"
#include "td/utils/base64.h"
#include "vm/boc.h"

#include "rANS.hpp"
#include "dictionary.hpp"
#include "utils.h"
#include "compressor.hpp"
#include "huffman.hpp"
#include "serializator.hpp"


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
    //
    //     // auto compressed = LZ_inf_compressor::compress(std::basic_string<uint8_t>(base64_data.begin(), base64_data.end()));
    //     // auto decompressed_ = LZ_inf_compressor::decompress(compressed);
    //     // std::string decompressed(decompressed_.begin(), decompressed_.end());
    //
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
        // std::cout << base64_data << std::endl;
    } else {
        // data = decompress(data);
        std::cout << Serializator::decompress(base64_data) << std::endl;
    }
    return 0;

    // std::cout << td::base64_encode(data) << std::endl;
}
