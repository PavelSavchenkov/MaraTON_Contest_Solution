#include <string>
#include <fstream>

#include "td/utils/check.h"
#include "utils.h"
#include "rANS.hpp"
#include "serializator.hpp"

int main() {
    const std::string pref = "/Users/pavel/Programming/MaraTON/ton-sample-tests/";
    const std::string files_names_file = pref + "file_list.txt";

    std::basic_string<uint8_t> data;
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

        const auto S_ = Serializator::compress(base64_data, false, false);
        // std::basic_string<uint8_t> S(reinterpret_cast<const uint8_t *>(S_.data()), S_.size());
        // data += S;
    }
    // auto probs = rANS::get_probs(data);
    //
    // std::cout << "Probabilities:" << std::endl;
    // std::cout << "{";
    // for (unsigned c = 0; c < rANS::context_vals; ++c) {
    //     std::cout << probs[c] * 1.0 / rANS::probs_interval << ",";
    // }
    // std::cout << "}" << std::endl;
}
