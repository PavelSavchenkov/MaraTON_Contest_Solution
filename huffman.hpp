#pragma once

#include <vector>

#include "utils.h"

namespace huffman {
static const uint8_t HUFFMAN_BITS = 8;
static const unsigned HUFFMAN_SIZE = 1u << HUFFMAN_BITS;

const std::array<unsigned, HUFFMAN_SIZE> CNT = {
    4497568, 3009243, 1011910, 859082, 882110, 747591, 771752, 675047, 890478, 615149, 601370, 615841, 795850, 614079,
    520080, 478994, 575699, 755142, 424570, 470626, 463391, 408715, 458294, 427150, 485664, 447284, 431932, 422242,
    408463, 401920, 381031, 401794, 541912, 396761, 1484487, 384743, 360771, 349257, 347622, 345042, 591869, 349195,
    337681, 334849, 354731, 345797, 348880, 361275, 444893, 408526, 389965, 457539, 401983, 398711, 387512, 373544,
    394810, 377759, 362281, 342651, 336170, 322140, 355486, 357122, 626789, 449486, 358318, 406953, 378766, 335164,
    340071, 338624, 710847, 345042, 330256, 316100, 389714, 328683, 331452, 336988, 456092, 342462, 393363, 354291,
    364484, 338436, 351397, 329438, 352655, 340826, 330885, 344098, 329375, 316288, 319434, 343658, 418341, 362659,
    364106, 347118, 377822, 346112, 374236, 335227, 572868, 324719, 348628, 309179, 343343, 332584, 324845, 350830,
    406009, 344727, 404185, 355046, 355990, 331200, 337303, 326292, 347496, 323084, 321259, 327236, 338121, 313268,
    317421, 326229, 504225, 385184, 391727, 369202, 386442, 344287, 351963, 334723, 862983, 341644, 334723, 322140,
    341581, 331577, 345294, 328306, 402234, 327173, 328620, 335667, 321574, 314086, 319749, 329690, 350642, 323964,
    331389, 321951, 336296, 324090, 330571, 323901, 668378, 393866, 377508, 353850, 353536, 336737, 356367, 353095,
    524987, 334786, 351019, 321385, 346237, 339631, 328809, 339757, 371279, 348565, 345105, 317987, 329879, 315911,
    321951, 312010, 347622, 354605, 340826, 313960, 342274, 358003, 421613, 340071, 753569, 431932, 388141, 362533,
    346552, 330634, 343280, 338687, 545813, 377885, 334346, 364232, 350201, 320693, 342399, 337177, 392230, 325223,
    321511, 324782, 337743, 323964, 323839, 326859, 325915, 319120, 309682, 314401, 303264, 313017, 316918, 314338,
    509195, 358003, 357940, 328432, 328494, 317295, 312639, 317484, 535306, 309682, 313017, 307165, 314590, 322014,
    309179, 320630, 563997, 423186, 384932, 434889, 390595, 337806, 314904, 333402, 399655, 326796, 352089, 326670,
    329753, 307354, 313268, 641260,
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
