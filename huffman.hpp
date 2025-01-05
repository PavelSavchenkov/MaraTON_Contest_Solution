#pragma once

#include <vector>

#include "utils.h"

namespace huffman {
static const uint8_t HUFFMAN_BITS = 8;
static const unsigned HUFFMAN_SIZE = 1u << HUFFMAN_BITS;

const std::array<unsigned, HUFFMAN_SIZE> CNT = {
    4950600, 2802815, 1974754, 1853132, 1711327, 1556238, 1642839, 1434444, 1702011, 1287463, 1441344, 1307819, 1613512,
    1258998, 1289015, 1215180, 1756697, 1242782, 1169981, 1135306, 1331971, 1106669, 1171707, 1230188, 1637319, 1222425,
    1146865, 1045945, 1167394, 950200, 946405, 1044047, 2075329, 1169981, 994536, 930361, 981080, 900861, 993328,
    935882, 1202414, 931396, 902759, 887750, 952443, 881022, 931051, 929154, 1443932, 1058366, 977802, 935019, 957791,
    908107, 903622, 882920, 1094248, 902932, 897929, 838757, 927083, 857561, 966761, 904657, 2558710, 1305749, 1005922,
    972972, 973317, 871879, 931741, 866876, 1143069, 859803, 856871, 825646, 966589, 871189, 877055, 835997, 1252615,
    966244, 903449, 857906, 884990, 848072, 869464, 844277, 996088, 858251, 840137, 834444, 894478, 769407, 847037,
    819090, 1676652, 1086140, 929499, 888440, 938469, 838584, 908970, 846865, 1076997, 832374, 875502, 836687, 919838,
    826508, 892581, 794076, 1222598, 917768, 910177, 809775, 869464, 831684, 833754, 803564, 949855, 821333, 841172,
    790798, 926048, 857906, 889821, 836859, 2876823, 1455318, 1120988, 996261, 955720, 911902, 910695, 887233, 1117365,
    875847, 893098, 848935, 969694, 862564, 842380, 838929, 1333869, 935709, 840654, 880160, 879987, 813397, 834789,
    804599, 1010925, 875675, 875157, 803047, 926221, 816675, 827026, 865324, 1705979, 1078377, 908107, 829096, 932431,
    806842, 882403, 817020, 1057503, 827026, 830649, 811500, 876192, 798734, 822368, 795974, 1123748, 913110, 855491,
    795629, 873087, 815640, 825818, 806497, 939159, 827543, 837722, 756296, 853593, 821506, 849625, 834444, 2123977,
    1349567, 1020758, 863081, 900689, 863771, 925358, 830131, 1122713, 888095, 853938, 816503, 931224, 832374, 835134,
    843415, 1239159, 905347, 837549, 812707, 874294, 813742, 832201, 797181, 951580, 807877, 819090, 774582, 820470,
    796836, 821506, 774927, 1639906, 1080965, 883438, 802529, 923978, 827888, 842380, 798734, 1060953, 787348, 831684,
    780792, 854110, 803047, 792868, 773029, 1216215, 874812, 877227, 800804, 892236, 778550, 799769, 771477, 1057331,
    845485, 872224, 767336, 927946, 776480, 858423, 1012477,
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
