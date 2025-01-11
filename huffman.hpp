#pragma once

#include <vector>

#include "utils.h"

namespace huffman {
static const std::vector<unsigned> d1_freq = {4894, 6201, 36038, 1867, 121, 0, 0, 0, 14591, 6, 25, 0, 0, 0, 0, 0,};
static const std::vector<unsigned> bytes_freq = {
    60529, 20038, 14391, 11912, 11143, 9046, 9527, 7541, 10147, 6581, 7090, 6648, 8459, 6052, 6504, 5627, 9745, 5967,
    5500, 5347, 6102, 5521, 5556, 5569, 7868, 5417, 5365, 4936, 5912, 4814, 5064, 5021, 10763, 5766, 5303, 5157, 5361,
    4472, 4954, 4574, 6303, 4595, 4706, 4549, 5287, 4594, 4768, 4752, 8303, 5125, 4913, 4916, 5277, 4482, 4580, 4363,
    5910, 4480, 4481, 4206, 4710, 4210, 4478, 4513, 14010, 5542, 5370, 5178, 5270, 4599, 5017, 4325, 5564, 4280, 4308,
    4196, 4979, 4288, 4375, 4177, 7051, 4536, 4347, 4228, 4782, 4127, 4442, 3997, 5591, 4302, 4538, 3947, 4766, 3934,
    4419, 4279, 9404, 4942, 5046, 4494, 4929, 4075, 4443, 4249, 5675, 4151, 4396, 4092, 4640, 3956, 4134, 4003, 6627,
    4339, 4589, 4091, 4437, 3998, 4086, 3913, 5145, 4081, 4132, 3981, 4685, 3965, 4336, 4181, 19966, 6118, 5496, 5111,
    5644, 4808, 5391, 4492, 5733, 4481, 4566, 4386, 4968, 4239, 4328, 4085, 6559, 4467, 4338, 4225, 4526, 4096, 4232,
    4091, 5390, 4204, 4277, 3982, 4592, 3932, 4111, 3990, 8898, 4768, 4652, 4249, 4644, 4033, 4383, 3863, 5182, 3923,
    4150, 3837, 4419, 3737, 4017, 3990, 6307, 4227, 4309, 3909, 4592, 4009, 3973, 3663, 5062, 4108, 4030, 3716, 4353,
    3847, 4082, 3959, 12592, 5432, 5074, 4530, 4986, 4357, 4367, 4111, 5399, 4252, 4226, 4181, 4722, 3871, 4228, 4002,
    6285, 4346, 4153, 3907, 4605, 3890, 3950, 3914, 4995, 3916, 4100, 3867, 4379, 3750, 3954, 3854, 8125, 4697, 4453,
    4073, 4543, 4195, 4091, 3900, 4967, 4095, 4012, 3853, 4345, 3893, 3827, 3722, 6113, 4134, 4087, 3907, 4412, 3838,
    3959, 3730, 5098, 4050, 4254, 3854, 4680, 3938, 4205, 11158,
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

    unsigned encode_symbol(td::BitPtr& bit_ptr, const unsigned s) const {
        const auto& code = codes[s];
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
            // if (freq[s] > 0) {
            nodes.insert(std::make_shared<Node>(s, freq[s]));
            sum_freq += freq[s];
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
};

std::basic_string<uint8_t> encode_8(const std::basic_string<uint8_t>& data) {
    HuffmanEncoderDecoder encoder(bytes_freq);

    std::basic_string<uint8_t> result(data.size() * 2, 0);
    td::BitPtr bits(result.data(), 0);
    for (const auto byte : data) {
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
