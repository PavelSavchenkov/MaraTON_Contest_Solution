#pragma once

#include <vector>

#include "utils.h"

namespace huffman {
static const uint8_t HUFFMAN_BITS = 8;
static const unsigned HUFFMAN_SIZE = 1u << HUFFMAN_BITS;

const std::array<unsigned, HUFFMAN_SIZE> CNT = {
    4474880, 1865837, 977117, 875998, 895988, 793506, 807655, 763586, 1043318, 745672, 609311, 596655, 763261, 600484,
    516045, 570369, 623070, 1046239, 428945, 412394, 445106, 393897, 419664, 389743, 469444, 414601, 409214, 398959,
    392015, 375140, 346323, 388705, 471586, 361186, 347751, 327890, 350801, 311989, 310107, 304850, 523833, 365015,
    309069, 304525, 320427, 318220, 305823, 328799, 447183, 417457, 382799, 377476, 390652, 514292, 357292, 351970,
    409993, 369364, 328929, 304201, 317506, 301605, 323672, 329188, 557778, 362679, 336393, 333472, 360472, 310821,
    308355, 306927, 526429, 318479, 305953, 300177, 358006, 304460, 301085, 328085, 425570, 330811, 320621, 313612,
    332174, 316662, 320946, 299009, 338729, 362030, 306148, 350542, 314196, 287845, 289468, 323153, 388640, 335419,
    333083, 320102, 350022, 333277, 337366, 306018, 514747, 297646, 318999, 287001, 341520, 305369, 301864, 330487,
    380916, 323347, 371700, 310431, 324580, 299463, 301540, 297451, 319778, 297581, 294206, 297905, 318155, 296023,
    300501, 302513, 418236, 351191, 345479, 334770, 353592, 317376, 311665, 304460, 509360, 312379, 302968, 295309,
    333602, 305109, 317636, 303097, 383772, 309847, 311924, 308355, 306797, 300436, 293427, 296997, 330357, 290896,
    302708, 298100, 309782, 300761, 302189, 313677, 497612, 358655, 331720, 317571, 327631, 305434, 323153, 311535,
    484242, 306407, 303292, 299138, 326073, 298749, 298424, 310496, 357811, 321984, 319648, 297840, 323023, 292453,
    300891, 288949, 319064, 314066, 308809, 288299, 312573, 334640, 380073, 314066, 540838, 375919, 343013, 325035,
    318934, 295893, 306927, 313222, 493134, 330097, 303487, 323802, 320102, 292064, 290571, 308744, 379359, 309523,
    296737, 301021, 326527, 299203, 293686, 304460, 321919, 292129, 286677, 292064, 287845, 287521, 288884, 280576,
    426154, 325165, 321660, 299982, 304395, 292323, 287910, 292908, 480867, 287326, 286677, 284405, 299009, 292908,
    285184, 292713, 1258667, 499170, 376503, 348335, 367871, 317895, 304525, 301150, 366378, 295179, 313157, 301475,
    318285, 287456, 289987, 474507,
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
