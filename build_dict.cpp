#include <string>
#include <fstream>

#include "td/utils/check.h"

#include "dictionary.hpp"
#include "serializator.hpp"

int main() {
    const std::string pref = "/Users/pavel/Programming/MaraTON/ton-sample-tests/";
    const std::string files_names_file = pref + "file_list.txt";

    Dictionary<2> dict{};
    unsigned total_bytes = 0;
    std::ifstream files_names(files_names_file);
    std::string line;
    while (files_names >> line) {
        if (line.empty() || line[0] != '1') {
            continue;
        }
        // Timer timer("suff auto");
        line = pref + line;
        // std::cout << "Found file = " << line << std::endl;
        std::ifstream cur_file(line);
        std::string mode;
        cur_file >> mode;
        CHECK(mode == "compress");

        std::string base64_data;
        cur_file >> base64_data;
        CHECK(!base64_data.empty());

        const auto S_bytes = Serializator::compress(base64_data, false, true);
        // std::basic_string<uint8_t> S_bytes;
        // S_bytes.push_back(125);
        const auto S_bits = bytes_str_to_bit_str(S_bytes);
        // std::basic_string<uint8_t> S_bits{1, 0, 1};

        total_bytes += (S_bits.size() + 7) / 8;
        std::cout << "training on " << line << ", " << S_bits.size()
                << " bits, total bytes = " << total_bytes << std::endl;
        dict.add_training_data(S_bits);
    }

    const auto dict_bits = dict.build_dict(
        (1 << 19),
        lz_compressor_bits::MIN_MATCH_LENGTH
    );
    std::cout << "dict length in bits: " << dict_bits.size() << std::endl;

    // const auto dict_packed = pack_expanded_bits_to_bytes<std::basic_string<uint8_t> >();

    std::basic_string<uint8_t> dict_lz = lz_compressor_bits::compress(dict_bits, {});
    std::cout << "dict length (bytes) after lz compression: " << dict_lz.size() << std::endl;

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
