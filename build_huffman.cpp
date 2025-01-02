#include "huffman.hpp"
#include "serializator.hpp"

int main() {
    const std::string pref = "/Users/pavel/Programming/MaraTON/ton-sample-tests/";
    const std::string files_names_file = pref + "file_list.txt";

    std::array<double, huffman::HUFFMAN_SIZE> cnt{};

    std::ifstream files_names(files_names_file);
    std::string line;
    while (files_names >> line) {
        if (line.empty() || line[0] != '1') {
            continue;
        }
        line = pref + line;
        std::cout << "Found file = " << line << std::endl;
        std::ifstream cur_file(line);
        // start encoding
        {
            std::string mode;
            cur_file >> mode;
            CHECK(mode == "compress");

            std::string base64_data;
            cur_file >> base64_data;
            CHECK(!base64_data.empty());

            td::BufferSlice data(td::base64_decode(base64_data).move_as_ok());
            td::Ref<vm::Cell> root = vm::std_boc_deserialize(data).move_as_ok();

            {
                auto S = Serializator::serialize(root);
                S = LZ_compressor::compress(S);
                // S = LZ_compressor::compress_standard(S);
                std::cout << "take encoded data from " << line << std::endl;

                unsigned bits_ptr = 0;
                unsigned number = 0;
                while (bits_ptr < S.size() * 8) {
                    uint8_t bit = (S[bits_ptr / 8] >> (7 - bits_ptr % 8)) & 1;
                    if (bit) {
                        number ^= 1u << (huffman::HUFFMAN_BITS - 1 - bits_ptr % huffman::HUFFMAN_BITS);
                    }
                    ++bits_ptr;
                    if (bits_ptr % huffman::HUFFMAN_BITS == 0) {
                        cnt[number] += 1.0;
                        number = 0;
                    }
                }
            }
        }
    }

    double min_cnt = std::numeric_limits<double>::max();
    double max_cnt = 0;
    for (unsigned i = 0; i < huffman::HUFFMAN_SIZE; ++i) {
        min_cnt = std::min(min_cnt, cnt[i]);
        max_cnt = std::max(max_cnt, cnt[i]);
    }

    std::array<unsigned, huffman::HUFFMAN_SIZE> cnt_int{};
    unsigned R = (1 << 30) / huffman::HUFFMAN_SIZE;
    const double coef = 1.0 / (max_cnt - min_cnt) * R;
    for (unsigned i = 0; i < huffman::HUFFMAN_SIZE; ++i) {
        cnt_int[i] = cnt[i] * coef;
    }

    huffman::output_freq(cnt_int);
}
