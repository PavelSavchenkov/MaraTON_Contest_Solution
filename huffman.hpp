#pragma once

#include <vector>

#include "utils.h"

namespace huffman {
static const std::vector<unsigned> d1_freq = {4894, 6201, 36038, 1867, 121, 0, 0, 0, 14591, 6, 25, 0, 0, 0, 0, 0,};

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

    void encode_symbol(td::BitPtr &bit_ptr, const unsigned s) const {
        // {
        //     bit_ptr.store_uint(s, 4);
        //     bit_ptr.offs += 4;
        //     return;
        // }

        const auto &code = codes[s];
        CHECK(code.second > 0);
        bit_ptr.store_uint(code.first, code.second);
        bit_ptr.offs += code.second;
    }

    unsigned decode_symbol(td::BitPtr &bit_ptr) const {
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

    explicit HuffmanEncoderDecoder(const std::vector<unsigned> &freq, bool verbose = false) {
        // build a tree
        auto cmp = [&](const Ptr<Node> &a, const Ptr<Node> &b) {
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
        auto dfs = [&](auto &&self, Ptr<Node> v, uint64_t code, uint8_t code_length) {
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
            std::cout << "huffman bits per symbol weighted: " << sum_code_lengths_weighted * 1.0 / sum_freq << std::endl;
            std::cout << "cnt symbols with freq > 0: " << cnt_alive_symbols << " out of " << freq.size() << std::endl;
            std::cout << "max_code_length: " << int(max_code_length) << std::endl;
        }
    }
};
};
