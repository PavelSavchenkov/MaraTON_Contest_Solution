#pragma once

#include <vector>

#include "utils.h"

namespace huffman {
static const uint8_t HUFFMAN_BITS = 8;
static const unsigned HUFFMAN_SIZE = 1u << HUFFMAN_BITS;

const std::array<unsigned, HUFFMAN_SIZE> CNT = {
    4708070, 2703666, 1601059, 1503763, 1420400, 1270254, 1388045, 1202780, 1474632, 1007383, 1016940, 1026151, 1292361,
    966392, 862649, 727356, 924480, 713654, 676808, 679687, 735301, 657119, 696037, 670130, 863225, 737719, 693389,
    683487, 687517, 624303, 617510, 626146, 878769, 631442, 616013, 588955, 607953, 573986, 592294, 579628, 925517,
    592409, 567308, 564544, 587803, 568344, 560284, 567308, 814289, 687632, 647677, 632018, 648598, 609450, 626721,
    609795, 670936, 617740, 565696, 540825, 568805, 542667, 579628, 602887, 1068754, 627182, 590682, 596093, 619122,
    560054, 576059, 563393, 949351, 601620, 553145, 533801, 640769, 555333, 569611, 552569, 729774, 566271, 567538,
    568459, 581355, 552915, 560629, 548194, 598396, 551303, 553721, 566732, 566617, 522978, 537025, 550267, 777673,
    617395, 633975, 602887, 654010, 585961, 619928, 568574, 914348, 553491, 566041, 526547, 596554, 561205, 547849,
    555103, 677154, 570992, 668748, 558211, 580895, 544279, 547964, 531959, 599317, 546006, 540825, 538867, 581240,
    523323, 583082, 570877, 995293, 739216, 650326, 653089, 639732, 594597, 575713, 585615, 947624, 597130, 566962,
    559823, 624303, 570647, 560169, 563278, 729774, 570762, 577555, 579283, 553145, 538867, 542782, 540825, 628103,
    548655, 564084, 559363, 583313, 546352, 559593, 571453, 905942, 639732, 611407, 580779, 611983, 561666, 593100,
    582737, 868982, 557981, 547043, 541401, 578246, 556024, 542207, 557405, 638005, 593906, 579283, 542091, 561090,
    524935, 540940, 536449, 597821, 577440, 548655, 522172, 560745, 609450, 683141, 589530, 1113314, 725859, 648368,
    603232, 610141, 562932, 599202, 565005, 921947, 605420, 558787, 562126, 584809, 534607, 534031, 563853, 663797,
    545200, 544625, 539673, 578246, 548194, 537601, 535413, 580779, 541285, 526547, 534377, 527814, 520099, 525971,
    522863, 835705, 604038, 597360, 551879, 573641, 534953, 525626, 536104, 864376, 525511, 528505, 513766, 543819,
    527468, 519408, 527008, 662646, 553145, 584003, 535298, 575137, 528274, 523784, 530232, 676578, 534147, 540940,
    530232, 604269, 522287, 553951, 683717,
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
