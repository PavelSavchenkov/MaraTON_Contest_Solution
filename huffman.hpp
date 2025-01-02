#pragma once

#include <vector>

#include "utils.h"

namespace huffman {
static const uint8_t HUFFMAN_BITS = 8;
static const unsigned HUFFMAN_SIZE = 1u << HUFFMAN_BITS;

const std::array<unsigned, HUFFMAN_SIZE> CNT = {
    4444447, 2616854, 917944, 744898, 751276, 661254, 684902, 599847, 831478, 606563, 531046, 537876, 669889, 525459,
    452481, 472687, 471220, 489788, 377811, 380576, 397170, 360032, 390228, 378545, 413142, 397396, 372957, 361725,
    345188, 333900, 313074, 354896, 406031, 331248, 320016, 306188, 305229, 292078, 284233, 277065, 481322, 306357,
    275767, 270913, 277460, 280000, 279548, 295182, 366241, 341068, 348857, 329667, 331868, 358508, 313525, 313187,
    348801, 328369, 284289, 274694, 273453, 270913, 299189, 300488, 504068, 317815, 292586, 295803, 309687, 280903,
    276952, 277347, 484878, 284346, 271534, 264874, 309800, 270010, 268768, 283951, 358170, 288409, 282427, 284854,
    293715, 278419, 280564, 265664, 297553, 298004, 274130, 301899, 277178, 263632, 260359, 285700, 328087, 300262,
    299810, 288014, 307543, 294956, 301729, 272832, 466591, 267075, 292247, 258609, 287845, 274977, 269050, 292360,
    331191, 286829, 333223, 283443, 288579, 268430, 270744, 264592, 281806, 264817, 263858, 267809, 276839, 261262,
    269389, 272493, 391469, 313525, 316968, 300149, 312509, 283499, 281185, 273396, 468680, 280113, 273001, 265043,
    287563, 279266, 283781, 271703, 319677, 271929, 276331, 279097, 271195, 264140, 263350, 266285, 292755, 261149,
    273114, 266285, 276557, 268599, 270123, 279548, 473477, 327523, 303818, 301842, 292868, 275089, 288184, 281016,
    442040, 287055, 273791, 266285, 282201, 272267, 268204, 279943, 298964, 285079, 290611, 267357, 275146, 261487,
    271816, 259004, 286773, 284910, 279943, 261036, 276444, 299189, 346317, 282201, 553792, 356420, 318549, 296367,
    281975, 269220, 277629, 284289, 455473, 303818, 275823, 300431, 282370, 262560, 268034, 284741, 327579, 273509,
    264987, 278363, 283894, 267583, 263858, 273396, 275033, 261262, 259286, 262221, 254489, 258948, 259456, 250143,
    384866, 292473, 295521, 273058, 273058, 264197, 257762, 262673, 446273, 257085, 260302, 256182, 260810, 269502,
    256069, 263237, 398073, 386333, 340673, 335819, 327466, 282427, 265551, 275936, 324531, 267414, 293715, 271703,
    270800, 258778, 260076, 581842,
};

struct Node {
    Ptr<Node> left{};
    Ptr<Node> right{};
    unsigned freq{};
    unsigned number{-1u};
    unsigned min_number{-1u};
    unsigned min_depth{-1u};

    explicit Node(unsigned number, unsigned freq) : number(number), freq(freq) {
        min_number = number;
        min_depth = 0;
    }

    explicit Node(Ptr<Node> left, Ptr<Node> right) : left(left), right(right) {
        freq = left->freq + right->freq;
        min_number = std::min(min_number, left->min_number);
        min_number = std::min(min_number, right->min_number);
        min_depth = std::min(left->min_depth, right->min_depth) + 1;
    }
};

Ptr<Node> build_tree(const std::array<unsigned, HUFFMAN_SIZE> &cnt) {
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
        CHECK(a->min_number != b->min_number);
        return a->min_number < b->min_number;
    };

    std::set<Ptr<Node>, decltype(cmp)> nodes(cmp);
    for (unsigned c = 0; c < HUFFMAN_SIZE; ++c) {
        nodes.insert(std::make_shared<Node>(c, cnt[c]));
    }
    while (nodes.size() > 1) {
        auto a = *nodes.begin();
        nodes.erase(nodes.begin());
        auto b = *nodes.begin();
        nodes.erase(nodes.begin());

        nodes.insert(std::make_shared<Node>(a, b));
    }
    const auto root = *nodes.begin();
    return root;
}

std::vector<std::vector<uint8_t> > build_codes(Ptr<Node> root) {
    std::vector<std::vector<uint8_t> > codes(HUFFMAN_SIZE);
    auto dfs = [&](auto &&self, Ptr<Node> v, std::vector<uint8_t> &code) {
        if (v->number != -1u) {
            codes.at(v->number) = code;
            return;
        }

        code.push_back(0);
        self(self, v->left, code);
        code.pop_back();

        code.push_back(1);
        self(self, v->right, code);
        code.pop_back();
    };

    // build codes for each byte via dfs
    {
        std::vector<uint8_t> code;
        dfs(dfs, root, code);
    }

    return codes;
}

void output_freq(const std::array<unsigned, HUFFMAN_SIZE> &cnt) {
    auto &s = std::cout;
    s << "const std::array<unsigned, HUFFMAN_SIZE> CNT = {\n";
    for (const auto &c: cnt) {
        s << c << ", ";
    }
    s << "\n};\n";
}

// Ptr<Node> ROOT{};

std::basic_string<uint8_t> encode(const std::basic_string<uint8_t> &data) {
    // std::array<unsigned, 256> cnt{};
    // for (const auto &c: data) {
    //     ++cnt[c];
    // }

    auto cnt = CNT;

    // output_freq(cnt);
    // exit(0);

    auto root = build_tree(cnt);
    const auto codes = build_codes(root);

    // debug huffman codes
    // {
    //     uint64_t sum_len = 0;
    //     uint64_t sum_sq = 0;
    //     for (const auto &c: codes) {
    //         sum_len += c.size();
    //         sum_sq += c.size() * c.size();
    //     }
    //     double avg = sum_len * 1.0 / HUFFMAN_SIZE;
    //     std::cout << "avg code length = " << avg << std::endl;
    //     std::cout << "stddev = " << sqrt(sum_sq * 1.0 / HUFFMAN_SIZE - avg * avg) << std::endl;
    // }

    std::basic_string<uint8_t> out;
    out.push_back(0);
    unsigned out_bits_ptr = 0;
    auto push_number = [&](unsigned number) {
        CHECK(number < HUFFMAN_SIZE);
        for (const auto &bit: codes[number]) {
            CHECK(bit <= 1);
            if (bit) {
                out.back() ^= 1u << (7 - out_bits_ptr % 8);
            }
            ++out_bits_ptr;
            if (out_bits_ptr % 8 == 0) {
                out.push_back(0);
            }
        }
    };

    unsigned number = 0;
    unsigned bit_i = 0;
    while (bit_i < data.size() * 8) {
        uint8_t bit = (data[bit_i / 8] >> (7 - bit_i % 8)) & 1;
        if (bit) {
            number ^= 1u << (HUFFMAN_BITS - 1 - bit_i % HUFFMAN_BITS);
        }
        ++bit_i;
        if (bit_i % HUFFMAN_BITS == 0) {
            push_number(number);
            number = 0;
        }
    }
    number ^= 1u << (HUFFMAN_BITS - 1 - bit_i % HUFFMAN_BITS);
    push_number(number);
    out.back() ^= 1u << (7 - out_bits_ptr % 8);
    // std::cout << "encode: cnt_bits = " << out_bits_ptr << std::endl;
    return out;
}

std::basic_string<uint8_t> decode(std::basic_string<uint8_t> data) {
    const auto root = build_tree(CNT);
    const auto codes = build_codes(root);

    CHECK(data.back() != 0);
    const unsigned cnt_bits = (data.size() - 1) * 8 + (7 - td::count_trailing_zeroes32(data.back()));
    std::basic_string<uint8_t> out;
    out.push_back(0);
    unsigned out_bits_ptr = 0;
    auto push_number = [&](unsigned number) {
        for (uint8_t it = 0; it < HUFFMAN_BITS; ++it) {
            const unsigned bit_i = HUFFMAN_BITS - 1 - it;
            uint8_t bit = (number >> bit_i) & 1;
            if (bit) {
                out.back() ^= 1u << (7 - out_bits_ptr % 8);
            }
            ++out_bits_ptr;
            if (out_bits_ptr % 8 == 0) {
                out.push_back(0);
            }
        }
    };

    Ptr<Node> v = root;
    for (unsigned bit_i = 0; bit_i < cnt_bits; ++bit_i) {
        uint8_t bit = (data[bit_i / 8] >> (7 - bit_i % 8)) & 1;
        if (bit) {
            v = v->right;
        } else {
            v = v->left;
        }
        CHECK(v);
        if (v->number != -1u) {
            CHECK(!v->left);
            CHECK(!v->right);
            CHECK(v->number < HUFFMAN_SIZE);
            push_number(v->number);
            v = root;
        }
    }
    CHECK(v == root);

    unsigned last_bit = out_bits_ptr - 1;
    while (!((out[last_bit / 8] >> (7 - last_bit % 8)) & 1)) {
        --last_bit;
    }
    CHECK(last_bit % 8 == 0);
    CHECK(last_bit > 0);
    --last_bit;
    while (out.size() - 1 > last_bit / 8) {
        out.pop_back();
    }
    return out;
}

std::basic_string<uint8_t> encode_8(const std::basic_string<uint8_t> &data) {
    // std::array<unsigned, 256> cnt{};
    // for (const auto &c: data) {
    //     ++cnt[c];
    // }

    auto cnt = CNT;

    // output_freq(cnt);
    // exit(0);

    auto root = build_tree(cnt);
    const auto codes = build_codes(root);

    std::basic_string<uint8_t> out;
    uint8_t byte = 0;
    unsigned bits_ptr = 0;
    for (const auto &c: data) {
        for (const auto &bit: codes[c]) {
            if (bit) {
                byte ^= (1u << (7 - bits_ptr));
            }
            ++bits_ptr;
            if (bits_ptr == 8) {
                out.push_back(byte);
                bits_ptr = 0;
                byte = 0;
            }
        }
    }
    byte ^= (1u << (7 - bits_ptr));
    out.push_back(byte);
    return out;
}

std::basic_string<uint8_t> decode_8(std::basic_string<uint8_t> data) {
    const auto root = build_tree(CNT);
    const auto codes = build_codes(root);

    unsigned cnt_bits = data.size() * 8 - 1;
    unsigned trailing_zeros = td::count_trailing_zeroes32(data.back());
    CHECK(trailing_zeros <= 7);
    CHECK((data.back() >> trailing_zeros) & 1);
    data.back() ^= 1u << trailing_zeros;
    cnt_bits -= trailing_zeros;

    Ptr<Node> v = root;
    std::basic_string<uint8_t> out;
    for (unsigned bit_i = 0; bit_i < cnt_bits; ++bit_i) {
        uint8_t bit = (data[bit_i / 8] >> (7 - bit_i % 8)) & 1;
        if (bit) {
            v = v->right;
        } else {
            v = v->left;
        }
        CHECK(v);
        if (v->number != -1u) {
            CHECK(!v->left);
            CHECK(!v->right);
            CHECK(v->number < 256);
            out.push_back(v->number);
            v = root;
        }
    }

    return out;
}
};
