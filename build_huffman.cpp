#include "huffman.hpp"
#include "serializator.hpp"

int main() {
    const std::string pref = "/Users/pavel/Programming/MaraTON/ton-sample-tests/";
    const std::string files_names_file = pref + "file_list.txt";

    std::array<unsigned, 256> freq{};
    std::ifstream files_names(files_names_file);
    std::string line;
    while (files_names >> line) {
        if (line.empty() || line[0] != '1') {
            continue;
        }
        line = pref + line;
        // std::cout << "Found file = " << line << std::endl;
        std::ifstream cur_file(line);
        // start encoding
        {
            std::string mode;
            cur_file >> mode;
            CHECK(mode == "compress");

            std::string base64_data;
            cur_file >> base64_data;
            CHECK(!base64_data.empty());

            const auto S_bytes = Serializator::compress(base64_data, true, false);
            for (const auto byte: S_bytes) {
                ++freq[byte];
            }
        }
    }

    std::cout << "static const std::vector<unsigned> bytes_freq = {";
    for (unsigned i = 0; i < 256; ++i) {
        std::cout << freq[i] << ", ";
    }
    std::cout << "};" << std::endl;
}
