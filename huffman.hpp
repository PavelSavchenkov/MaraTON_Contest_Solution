#pragma once

#include <vector>

#include "utils.h"

namespace huffman {
static const std::vector<unsigned> d1_freq = {4894, 6201, 36038, 1867, 121, 0, 0, 0, 14591, 6, 25, 0, 0, 0, 0, 0,};
static const std::vector<unsigned> data_size_freq = {
    22, 23, 2058, 4, 88, 199, 75, 641, 53, 90, 31, 54, 4, 17, 2, 12, 13, 8, 4, 7, 2, 12, 7, 5, 4, 4, 4, 1, 1, 0, 2, 760,
    43, 250, 44, 168, 103, 51, 85, 677, 101, 107, 35, 72, 3564, 29, 284, 56, 86, 16, 43, 5, 6927, 8, 90, 15, 30, 119,
    20, 31, 8818, 11, 90, 10, 76, 85, 2133, 432, 4513, 183, 136, 46, 49, 47, 60, 153, 504, 17, 10, 29, 111, 1, 6, 19, 5,
    1, 5, 211, 11, 0, 9, 0, 0, 4, 0, 197, 21, 22, 0, 0, 0, 11, 0, 57, 21, 2, 4, 0, 0, 0, 0, 35, 8, 0, 1, 0, 6, 0, 0, 3,
    13, 0, 0, 0, 1, 0, 7, 9, 23, 15, 27, 29, 32, 36, 9, 6, 10, 1, 1, 0, 1, 0, 0, 11, 2, 0, 0, 0, 5, 17, 0, 11, 9, 0, 1,
    0, 0, 1, 1, 0, 5, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 4, 46, 0, 0, 2, 0, 0, 0, 3,
    2, 0, 0, 0, 0, 0, 0, 4, 0, 2, 0, 8, 0, 0, 0, 1, 0, 0, 0, 4, 2, 0, 2, 1, 0, 16, 0, 6, 0, 0, 0, 5, 0, 0, 0, 8, 0, 0,
    0, 5, 0, 0, 0, 1, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 8, 2, 85, 0, 1, 7, 7, 24, 55, 37, 55, 87,
    52, 55, 50, 49, 35, 42, 18, 6, 0, 65, 2, 1, 6, 0, 14507, 5, 1, 1, 5, 5, 14, 20, 11, 13, 16, 21, 15, 14, 29, 37, 48,
    65, 64, 33, 19, 18, 19, 3, 0, 13, 3, 0, 0, 2, 6, 1, 0, 0, 1, 5, 2, 12, 64, 262, 167, 46, 35, 29, 18, 1, 3, 2, 0, 0,
    4, 2, 0, 8, 2, 2, 0, 0, 3, 1, 12, 2, 4, 10, 10, 17, 18, 15, 11, 10, 35, 41, 50, 57, 66, 44, 15, 5, 2, 0, 53, 5, 8,
    26, 1, 1, 6, 1, 0, 0, 0, 0, 0, 0, 28, 22, 0, 1, 0, 2, 2, 1, 3, 11, 4, 1, 16, 17, 20, 41, 55, 70, 0, 1, 12, 0, 0, 0,
    41, 0, 0, 1, 0, 4, 0, 5, 60, 9, 0, 11, 0, 3, 0, 6, 183, 1023, 0, 5, 0, 2, 1, 0, 9, 50, 0, 2, 0, 8, 5, 0, 11, 114, 0,
    2, 0, 2, 0, 2, 20, 333, 0, 3, 8, 503, 25, 37, 72, 216, 171, 149, 186, 101, 22, 0, 12, 133, 0, 0, 3, 1, 0, 6, 3, 34,
    2, 2, 3, 1, 0, 0, 8, 4, 0, 9, 2, 0, 0, 0, 7, 44, 2, 2, 1, 0, 0, 0, 17, 0, 0, 8, 2, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0,
    9, 0, 2, 0, 2, 2, 0, 5, 10, 0, 2, 0, 0, 1, 0, 4, 971, 0, 3, 5, 0, 25, 0, 0, 15, 0, 0, 0, 0, 0, 12, 0, 1, 0, 30, 0,
    1, 1, 88, 3, 2, 25, 0, 0, 3, 0, 0, 0, 5, 1, 0, 0, 1, 0, 3, 0, 2, 0, 2, 0, 0, 3, 13, 1, 8, 4, 2, 0, 0, 2, 19, 2, 2,
    0, 10, 0, 0, 1, 17, 1, 2, 2, 17, 0, 4, 9, 37, 18, 36, 28, 40, 4, 15, 20, 53, 52, 109, 74, 56, 11, 9, 12, 18, 30,
    134, 40, 19, 5, 10, 10, 22, 37, 66, 37, 17, 5, 5, 4, 0, 6, 111, 9, 82, 5, 1, 0, 10, 10, 348, 2, 5, 4, 5, 1, 4, 1,
    40, 6, 28, 18, 25, 29, 37, 71, 73, 116, 89, 46, 24, 2, 0, 0, 1, 1, 2, 2, 4, 2, 1, 1, 5, 1, 0, 1, 0, 0, 0, 17, 15, 3,
    3, 0, 7, 1, 5, 40, 32, 98, 50, 48, 73, 61, 36, 25, 25, 4, 0, 0, 0, 0, 0, 24, 21, 62, 23, 1, 36, 4, 5, 25, 29, 428,
    184, 1, 36, 0, 0, 8, 26, 32, 1, 0, 24, 0, 0, 2, 20, 0, 0, 0, 764, 0, 0, 13, 9, 27, 4, 0, 7, 0, 0, 0, 1, 18, 4, 4, 1,
    0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 4, 1, 0, 0, 0, 8, 4, 19, 2, 0, 0, 0, 1, 1, 3, 4, 2, 10,
    0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 16, 1, 21, 1, 3, 0, 0, 0, 0, 0, 1, 5, 141, 0, 2, 0, 10, 0, 0, 6, 4, 13, 6, 2, 0, 0, 0,
    5, 7, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 7, 1, 0, 0, 0, 2, 1, 0, 3, 0, 0, 0, 0, 1, 0, 0, 5, 0, 0, 0, 0, 3, 2,
    1, 4, 0, 6, 0, 0, 3, 3, 0, 4, 3, 0, 0, 21, 4, 3, 3, 1, 1, 3, 0, 2, 1, 18, 2, 8, 0, 0, 0, 0, 2, 2, 0, 23, 1, 0, 0, 0,
    4, 0, 0, 2, 31, 0, 1, 0, 96, 1, 2, 3, 1, 0, 7, 1, 0, 2, 0, 1, 8, 0, 0, 0, 0, 0, 29, 12, 0, 1, 0, 0, 7, 1, 0, 1, 1,
    19, 0, 0, 0, 1, 0, 7, 2, 15, 0, 0, 0, 0, 1, 7, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0, 3, 0, 6, 2, 0, 0,
    13, 0, 1, 0, 0, 0, 4, 0, 24, 1, 1, 0, 0, 1, 0, 1, 15, 1, 12, 0, 0, 1, 11, 0, 8, 6, 0, 0, 0, 1, 0, 1, 1, 11, 2, 0, 0,
    0, 0, 0, 5, 2, 0, 0, 0, 0, 0, 0, 24, 2, 0, 0, 0, 0, 0, 14,
};


static const std::vector<unsigned> bytes_freq = {
    64447, 19336, 13382, 11526, 10742, 8228, 9038, 8132, 9540, 6786, 7222, 5870, 8186, 5945, 6402, 5848, 9678, 5730,
    5871, 5364, 6906, 5249, 5177, 4914, 7402, 5549, 5390, 4919, 5855, 4939, 4926, 5274, 10452, 5607, 5486, 5133, 5657,
    4820, 5251, 4777, 7096, 4930, 4810, 4382, 5067, 4474, 4471, 4577, 7732, 4835, 5242, 4603, 4988, 4909, 4605, 4517,
    5822, 4636, 4659, 4503, 4815, 4345, 4755, 4584, 15194, 5338, 5226, 5024, 5270, 4751, 4925, 4387, 5621, 4667, 4885,
    4310, 5141, 4588, 4549, 4353, 8991, 4683, 4657, 4589, 5200, 4390, 4238, 4245, 5178, 4229, 4377, 4169, 4649, 4220,
    4253, 4383, 8620, 4906, 4749, 4385, 5238, 4473, 4473, 4139, 5060, 4241, 4835, 4002, 4638, 4056, 4307, 3928, 5915,
    4527, 4854, 4205, 4726, 4247, 4219, 3981, 4856, 4193, 4176, 4074, 4650, 4268, 4359, 4318, 18775, 5808, 5738, 5038,
    5461, 4840, 5092, 4624, 5656, 4680, 4927, 4226, 4974, 4543, 4449, 4310, 6594, 4533, 4560, 4369, 5470, 4287, 4339,
    4109, 5320, 4453, 4542, 4080, 4583, 4232, 4206, 4102, 9560, 4781, 4517, 4359, 4710, 4410, 4550, 4173, 5793, 4295,
    4471, 3985, 4491, 3860, 4096, 3926, 5756, 4334, 4416, 3954, 4299, 4075, 4040, 3736, 4795, 4183, 4388, 3846, 4401,
    3994, 4250, 4094, 10311, 5186, 5063, 4623, 4814, 4396, 4314, 4319, 5460, 4373, 4635, 4173, 4679, 4056, 4241, 4083,
    5897, 4313, 4389, 4127, 5195, 4185, 4101, 4131, 4885, 3988, 4175, 3904, 4422, 4002, 4057, 4040, 7085, 4627, 4549,
    4127, 4717, 4323, 4043, 3967, 5003, 4142, 4278, 3875, 4460, 3955, 3900, 4032, 5737, 4287, 4396, 3971, 4510, 4018,
    4121, 3967, 4887, 4265, 4416, 3941, 4628, 4125, 4316, 9840,
};


struct Node {
    Ptr<Node> left{};
    Ptr<Node> right{};
    unsigned freq{};
    unsigned symbol{-1u};
    unsigned min_symbol{-1u};
    unsigned min_depth{-1u};

    explicit Node(unsigned symbol, unsigned freq) : symbol(symbol), freq(freq) {
        min_symbol = symbol;
        min_depth = 0;
    }

    explicit Node(Ptr<Node> left, Ptr<Node> right) : left(left), right(right) {
        freq = left->freq + right->freq;
        min_symbol = std::min(min_symbol, left->min_symbol);
        min_symbol = std::min(min_symbol, right->min_symbol);
        min_depth = std::min(left->min_depth, right->min_depth) + 1;
    }
};

struct HuffmanEncoderDecoder {
    // 0 ... n -> bitstrings
    // (code, length)
    std::vector<std::pair<uint64_t, uint8_t> > codes{};
    Ptr<Node> root{};

    explicit HuffmanEncoderDecoder(const std::vector<unsigned>& freq, bool verbose = false) {
        // build a tree
        auto cmp = [&](const Ptr<Node>& a, const Ptr<Node>& b) {
            if (a == b) {
                return false;
            }
            if (a->freq != b->freq) {
                return a->freq < b->freq;
            }
            if (a->min_depth != b->min_depth) {
                return a->min_depth < b->min_depth;
            }
            CHECK(a->min_symbol != b->min_symbol);
            return a->min_symbol < b->min_symbol;
        };

        unsigned sum_freq = 0;
        std::set<Ptr<Node>, decltype(cmp)> nodes(cmp);
        for (unsigned s = 0; s < freq.size(); ++s) {
            const auto f = std::max(1u, freq[s]);
            // if (freq[s] > 0) {
            nodes.insert(std::make_shared<Node>(s, f));
            sum_freq += f;
            // }
        }
        const unsigned cnt_alive_symbols = nodes.size();
        CHECK(!nodes.empty());
        while (nodes.size() > 1) {
            auto a = *nodes.begin();
            nodes.erase(nodes.begin());
            auto b = *nodes.begin();
            nodes.erase(nodes.begin());

            nodes.insert(std::make_shared<Node>(a, b));
        }
        root = *nodes.begin();

        // calculate codes
        codes.resize(freq.size());
        unsigned sum_code_lengths = 0;
        unsigned sum_code_lengths_weighted = 0;
        uint8_t max_code_length = 0;
        auto dfs = [&](auto&& self, Ptr<Node> v, uint64_t code, uint8_t code_length) {
            if (v->symbol != -1u) {
                CHECK(code_length <= 64);
                codes[v->symbol] = {code, code_length};
                sum_code_lengths += code_length;
                sum_code_lengths_weighted += code_length * v->freq;
                max_code_length = std::max(max_code_length, code_length);
                return;
            }

            self(self, v->left, code * 2, code_length + 1);

            self(self, v->right, code * 2 + 1, code_length + 1);
        };

        dfs(dfs, root, 0, 0);

        if (verbose) {
            std::cout << "HUFFMAN stats: " << std::endl;
            std::cout << "naive bits per symbol: " << int(len_in_bits(freq.size() - 1)) << std::endl;
            std::cout << "huffman bits per symbol: " << sum_code_lengths * 1.0 / freq.size() << std::endl;
            std::cout << "huffman bits per symbol weighted: " << sum_code_lengths_weighted * 1.0 / sum_freq <<
                    std::endl;
            std::cout << "cnt symbols with freq > 0: " << cnt_alive_symbols << " out of " << freq.size() << std::endl;
            std::cout << "max_code_length: " << int(max_code_length) << std::endl;
        }
    }

    unsigned encode_symbol(td::BitPtr& bit_ptr, const unsigned s) const {
        const auto& code = codes[s];
        // std::cout << "s = " << s << std::endl;
        CHECK(code.second > 0);
        bit_ptr.store_uint(code.first, code.second);
        bit_ptr.offs += code.second;
        return code.second;
    }

    unsigned decode_symbol(td::BitPtr& bit_ptr) const {
        // {
        //     unsigned res = bit_ptr.get_uint(4);
        //     bit_ptr.offs += 4;
        //     return res;
        // }

        CHECK(root);
        Ptr<Node> v = root;
        while (v->symbol == -1u) {
            CHECK(v);
            bool bit = bit_ptr.get_uint(1);
            bit_ptr.offs += 1u;

            CHECK(v->left);
            CHECK(v->right);

            if (bit) {
                v = v->right;
            } else {
                v = v->left;
            }
        }
        return v->symbol;
    }
};

std::basic_string<uint8_t> encode_8(const std::basic_string<uint8_t>& data) {
    HuffmanEncoderDecoder encoder(bytes_freq);

    std::basic_string<uint8_t> result(data.size() * 2, 0);
    td::BitPtr bits(result.data(), 0);
    for (const auto byte: data) {
        encoder.encode_symbol(bits, byte);
    }
    bits.store_uint(1, 1);
    bits.offs += 1;
    const unsigned new_size = (bits.offs + 7) / 8;
    CHECK(result.size() >= new_size);
    result.resize(new_size);
    return result;
}

std::basic_string<uint8_t> decode_8(std::basic_string<uint8_t>& data) {
    HuffmanEncoderDecoder decoder(bytes_freq);

    std::basic_string<uint8_t> result;
    td::BitPtr bits(data.data(), 0);
    unsigned cnt_bits = data.size() * 8;
    CHECK(data.back() != 0);
    for (unsigned tail = 0; tail < 8; ++tail) {
        if ((data.back() >> tail) & 1) {
            cnt_bits -= (tail + 1);
            break;
        }
    }

    while (bits.offs < cnt_bits) {
        const auto symbol = decoder.decode_symbol(bits);
        result.push_back(symbol);
    }
    return result;
}
};
