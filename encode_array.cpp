#include "huffman.hpp"
#include "utils_local.hpp"

std::string encode(const std::vector<unsigned>& numbers) {
    uint8_t max_bits = 0;
    for (const auto& num : numbers) {
        max_bits = std::max(max_bits, len_in_bits(num));
    }
    CHECK(max_bits <= 32);

    std::basic_string<uint8_t> buff(4 + (max_bits * numbers.size() + 7) / 8 + 10, 0);
    td::BitPtr bit_ptr(buff.data(), 0);

    bit_ptr.store_uint(numbers.size(), 4 * 8);
    bit_ptr.offs += 4 * 8;

    bit_ptr.store_uint(max_bits, 5);
    bit_ptr.offs += 5;

    for (const auto& num : numbers) {
        bit_ptr.store_uint(num, max_bits);
        bit_ptr.offs += max_bits;
    }
    buff.resize((bit_ptr.offs + 7) / 8);
    buff = compress_lz_standard(buff);
    const auto base64 = td::base64_encode(td::Slice(buff.data(), buff.size()));
;   return base64;
}

void work(const std::vector<unsigned>& v, const std::string& name) {
    const auto s = encode(v);
    std::cout << "static const std::string " << name << " = \"" << s << "\";" << std::endl;

    // const auto decoded = decode_vector(huffman::offset_within_cell_freq_str);
    // // const auto decoded = decode_vector(s);
    // CHECK(decoded == huffman::offset_within_cell_freq);
    // std::cout << "success!" << std::endl;
}
int main() {
    // work(huffman::offset_within_cell_freq, "offset_within_cell_freq_str");
    work(huffman::data_size_freq, "data_size_freq_str");
}
