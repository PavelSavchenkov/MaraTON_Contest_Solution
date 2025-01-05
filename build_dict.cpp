#include <string>
#include <fstream>

#include "td/utils/check.h"

#include "dictionary.hpp"
#include "serializator.hpp"

int main() {
    const std::string pref = "/Users/pavel/Programming/MaraTON/ton-sample-tests/";
    const std::string files_names_file = pref + "file_list.txt";

    Dictionary dict;
    unsigned total_bytes = 0;
    std::ifstream files_names(files_names_file);
    std::string line;
    while (files_names >> line) {
        if (line.empty() || line[0] != '1') {
            continue;
        }
        line = pref + line;
        // std::cout << "Found file = " << line << std::endl;
        std::ifstream cur_file(line);
        std::string mode;
        cur_file >> mode;
        CHECK(mode == "compress");

        std::string base64_data;
        cur_file >> base64_data;
        CHECK(!base64_data.empty());

        const auto S_ = Serializator::compress(base64_data, false, true);
        std::basic_string<uint8_t> S(reinterpret_cast<const uint8_t *>(S_.data()), S_.size());

        total_bytes += S.size();
        std::cout << "training on " << line << ", " << S.size() << " bytes, total bytes = " << total_bytes << std::endl;
        dict.add_training_data(S);
    }

    std::basic_string<uint8_t> dict_bytes = dict.build_dict(1 << 17);
    std::cout << "dict length in bytes: " << dict_bytes.size() << std::endl;

    std::basic_string<uint8_t> dict_lz = LZ_compressor::compress(dict_bytes, 4);
    std::cout << "dict length after lz compression: " << dict_lz.size() << std::endl;

    std::string dict_base64 = td::base64_encode(
        td::BufferSlice(reinterpret_cast<const char *>(dict_lz.data()), dict_lz.size())
    );
    std::cout << "dict length after base 64: " << dict_base64.size() << std::endl;

    // output compressed dict
    {
        std::ofstream outfile("dicts/dict_base64.txt");
        outfile << dict_base64;
    }
}
