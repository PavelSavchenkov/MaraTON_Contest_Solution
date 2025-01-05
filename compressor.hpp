#pragma once

#include <numeric>
#include <set>
#include <string>
#include <td/utils/check.h>

#include "utils.h"

namespace LZ_compressor {
static const uint8_t MAX_BYTES_FOR_INDEX = 3;
static const unsigned MATCH_OFFSET = 4;
static const uint8_t LITERAL_NIBBLE_BITS = 6;
static const auto MATCH_NIBBLE_BITS = 8 - LITERAL_NIBBLE_BITS;

std::vector<unsigned> build_suff_arr(const std::basic_string<uint8_t> &s) {
    const unsigned n = s.size();
    std::vector<unsigned> p(n);

    const unsigned MAX = *std::max_element(s.begin(), s.end());
    const unsigned C = std::max(MAX + 1, n);

    std::vector<unsigned> cnt(C, 0);
    for (const auto &ch: s) {
        ++cnt[ch];
    }
    for (unsigned i = 0; i + 1 < C; ++i) {
        cnt[i + 1] += cnt[i];
    }
    for (unsigned i = 0; i < n; ++i) {
        p[--cnt[s[i]]] = i;
    }

    std::vector<unsigned> c(C);
    unsigned cls = 1;
    c[p[0]] = cls - 1;
    for (unsigned i = 1; i < n; ++i) {
        if (s[p[i]] != s[p[i - 1]]) {
            ++cls;
        }
        c[p[i]] = cls - 1;
    }

    std::vector<unsigned> nc(C);
    std::vector<unsigned> np(n);
    for (unsigned len = 1; len <= n; len *= 2) {
        std::fill(std::begin(cnt), std::begin(cnt) + cls, 0);
        for (unsigned i = 0; i < n; ++i) {
            ++cnt[c[i]];
        }
        for (unsigned i = 0; i + 1 < cls; ++i) {
            cnt[i + 1] += cnt[i];
        }
        for (unsigned ii = 0; ii < n; ++ii) {
            unsigned i = n - 1 - ii;
            const unsigned j = p[i];
            unsigned j2 = (j + n - len) % n;
            np[--cnt[c[j2]]] = j2;
        }
        p = np;

        cls = 1;
        nc[p[0]] = cls - 1;
        for (unsigned i = 1; i < n; ++i) {
            if (c[p[i]] != c[p[i - 1]] || c[(p[i] + len) % n] != c[(p[i - 1] + len) % n]) {
                ++cls;
            }
            nc[p[i]] = cls - 1;
        }
        c = nc;
    }

    return p;
}

std::basic_string<uint8_t> compress_standard(const std::basic_string<uint8_t> &data) {
    auto res = td::lz4_compress(td::BufferSlice(reinterpret_cast<const char *>(data.data()), data.size()));
    return {reinterpret_cast<const uint8_t *>(res.data()), res.size()};
}

struct CompressionPrecalc {
    std::vector<unsigned> p{};
    std::vector<unsigned> pos{};
    unsigned n{};

    std::vector<unsigned> best_len{};
    std::vector<unsigned> best_prev{};

    explicit CompressionPrecalc() = default;

    void precalc(const std::basic_string<uint8_t> &data) {
        // sort cyclic shifts
        p = build_suff_arr(data);

        n = p.size();
        CHECK(n == data.size());

        pos.resize(n);
        for (unsigned i = 0; i < n; ++i) {
            pos[p[i]] = i;
        }

        // {
        //     auto data2 = data + data;
        //     for (unsigned i = 0; i + 1 < n; ++i) {
        //         unsigned p1 = p[i];
        //         unsigned p2 = p[i + 1];
        //         auto sub1 = data2.substr(p1, n);
        //         auto sub2 = data2.substr(p2, n);
        //         if (sub1 > sub2) {
        //             std::cout << "i = " << i << ", n = " << n << std::endl;
        //             std::cout << "p1 = " << p1 << ", p2 = " << p2 << std::endl;
        //             exit(0);
        //         }
        //     }
        // }

        // {
        //     std::vector<unsigned> my_p(n);
        //     std::iota(std::begin(my_p), std::end(my_p), 0);
        //     std::sort(std::begin(my_p), std::end(my_p), [&](const unsigned &a, const unsigned &b) {
        //         return data.substr(a, n - a) < data.substr(b, n - b);
        //     });
        //     std::cout << "p: " << std::endl;
        //     for (unsigned i = 0; i < n; ++i) {
        //         auto sub = data.substr(p[i], n - p[i]);
        //         std::cout << i << " p[i] = " << p[i] << " -> " << std::string(sub.begin(), sub.end()) << std::endl;
        //     }
        //     std::cout << "my_p: " << std::endl;
        //     for (unsigned i = 0; i < n; ++i) {
        //         auto sub = data.substr(my_p[i], n - my_p[i]);
        //         std::cout << i << " my_p[i] = " << my_p[i] << " -> " << std::string(sub.begin(), sub.end()) << std::endl;
        //     }
        //     CHECK(p == my_p);
        // }

        auto get_lcp = [&](unsigned j, unsigned i) -> unsigned {
            if (i < j) {
                std::swap(i, j);
            }
            const auto i0 = i;
            while (i < n && data[i] == data[j]) {
                ++i;
                ++j;
            }
            return i - i0;
        };

        std::set<unsigned> alive_pos;
        unsigned leftmost_alive = 0;
        uint8_t bytes_for_index = 1;
        std::map<uint64_t, unsigned> map_pref;
        auto make_alive = [&](unsigned i) {
            alive_pos.insert(pos[i]);
        };
        best_prev.resize(n);
        best_len.resize(n);
        for (unsigned i = 0; i < n;) {
            if (i >= (1u << (8 * bytes_for_index)) && bytes_for_index < MAX_BYTES_FOR_INDEX) {
                ++bytes_for_index;
            }

            while (leftmost_alive < i && len_in_bytes(i - leftmost_alive) > MAX_BYTES_FOR_INDEX) {
                auto it = alive_pos.find(pos[leftmost_alive]);
                CHECK(it != alive_pos.end());
                alive_pos.erase(it);
                ++leftmost_alive;
            }

            // find earlier suffix with the max lcp
            best_prev[i] = -1u;
            best_len[i] = 0;
            // to the left and to the right in suff arr
            for (unsigned phase = 0; phase < 2; ++phase) {
                auto it = alive_pos.upper_bound(pos[i]);
                if (phase == 1) {
                    if (it != alive_pos.begin()) {
                        --it;
                    }
                }
                if (it != alive_pos.end()) {
                    unsigned j = p[*it];
                    CHECK(j < i);
                    CHECK(len_in_bytes(i - j) <= bytes_for_index);
                    const auto len = get_lcp(j, i);
                    if (len > best_len[i]) {
                        best_prev[i] = j;
                        best_len[i] = len;
                    }
                }
            }

            make_alive(i++);
        }
    }

    bool empty() const {
        return p.empty();
    }
} MEM;

std::basic_string<uint8_t> compress(
    const std::basic_string<uint8_t> &data,
    unsigned MIN_MATCH_LENGTH,
    bool clear_mem = true
) {
    CHECK(!data.empty());
    CHECK(MATCH_OFFSET <= MIN_MATCH_LENGTH);

    if (MEM.empty()) {
        MEM.precalc(data);
    }
    CHECK(!MEM.empty());
    CHECK(MEM.n == data.size());

    // encoded size is at most data size ??
    std::basic_string<uint8_t> out_vector(2 << 20, 0);
    td::BitPtr out(out_vector.data(), 0);
    const auto store_uint = [&](unsigned x, unsigned bits) {
        out.store_uint(x, bits);
        out.offs += bits;
    };

    // number of bits to store offset size
    auto bits_for_offset = static_cast<uint8_t>(-1u);
    // minimal offset size. e.g. x in "bits for offset" field means the offset takes "x + min_bits_for_offset" bits
    auto min_bits_for_offset = static_cast<uint8_t>(-1u);
    {
        // offset size can be up to data.size()-1
        const uint8_t bits_for_max_offset = len_in_bits(data.size() - 1); // e.g. 20
        bits_for_offset = len_in_bits(bits_for_max_offset); // e.g. 5
        min_bits_for_offset = 0;
        while (bits_for_offset > 4) {
            ++min_bits_for_offset;
            bits_for_offset = len_in_bits(bits_for_max_offset - min_bits_for_offset);
        }
    }

    // store both constants in out buf
    {
        CHECK(bits_for_offset <= 4);
        CHECK(min_bits_for_offset <= 15);
        uint8_t byte = (bits_for_offset << 4) ^ min_bits_for_offset;
        store_uint(byte, 8);
    }

    const auto push_offset = [&](unsigned offset) {
        const uint8_t offset_bit_length = std::max(min_bits_for_offset, len_in_bits(offset));
        store_uint(offset_bit_length - min_bits_for_offset, bits_for_offset);
        store_uint(offset, offset_bit_length);
    };

    std::basic_string<uint8_t> literal_buf;
    std::basic_string<uint8_t> all_literals;
    auto dump_int_number_per_255 = [&](unsigned number) -> void {
        while (1) {
            unsigned cur = std::min(number, 255u);
            number -= cur;
            store_uint(cur, 8);
            if (cur < 255) {
                CHECK(number == 0);
                break;
            }
        }
    };
    auto dump_tokens_and_literal = [&](unsigned match_length) {
        unsigned literal_length = literal_buf.size();
        unsigned literal_nibble = std::min(literal_length, (1u << LITERAL_NIBBLE_BITS) - 1);
        literal_length -= literal_nibble;

        uint8_t token = 0;
        token = literal_nibble << MATCH_NIBBLE_BITS;

        CHECK(match_length >= MIN_MATCH_LENGTH);
        if (match_length == -1u) {
            match_length = 0;
        } else {
            match_length -= MATCH_OFFSET;
        }
        unsigned match_nibble = std::min(match_length, (1u << MATCH_NIBBLE_BITS) - 1);
        match_length -= match_nibble;
        token ^= match_nibble;

        store_uint(token, 8);

        if (literal_nibble == (1u << LITERAL_NIBBLE_BITS) - 1) {
            dump_int_number_per_255(literal_length);
        }

        all_literals += literal_buf;
        literal_buf.clear();

        if (match_nibble == (1u << MATCH_NIBBLE_BITS) - 1) {
            dump_int_number_per_255(match_length);
        }
    };

    uint8_t bytes_for_index = 1;
    for (unsigned i = 0; i < MEM.n;) {
        if (i >= (1u << (8 * bytes_for_index)) && bytes_for_index < MAX_BYTES_FOR_INDEX) {
            ++bytes_for_index;
        }

        if (MEM.best_len[i] >= MIN_MATCH_LENGTH) {
            dump_tokens_and_literal(MEM.best_len[i]);

            // dump match offset
            const unsigned j = MEM.best_prev[i];
            CHECK(j < i);
            CHECK(len_in_bytes(i - j) <= MAX_BYTES_FOR_INDEX);

            for (unsigned it = 0; it < MEM.best_len[i]; ++it) {
                CHECK(data[j + it] == data[i + it]);
            }

            const unsigned offset = i - j;
            push_offset(offset);
            i += MEM.best_len[i];
        } else {
            literal_buf.push_back(data[i]);
            ++i;
        }
    }
    // if (!literal_buf.empty()) {
    //     dump_tokens_and_literal(-1u);
    // }
    // TODO: think more
    dump_tokens_and_literal(-1u);

    if (clear_mem) {
        MEM = CompressionPrecalc();
    }

    for (const auto& byte : all_literals) {
        store_uint(byte, 8);
    }
    store_uint(1, 1);

    const unsigned len_in_bytes = (out.offs + 7) / 8;
    return {out.ptr, len_in_bytes};
}

std::basic_string<uint8_t> decompress_standard(const std::basic_string<uint8_t> &data) {
    auto res = td::lz4_decompress(
        td::BufferSlice(reinterpret_cast<const char *>(data.data()), data.size()),
        2 << 20
    ).move_as_ok();
    return {reinterpret_cast<const uint8_t *>(res.data()), res.size()};
}

struct Token {
    unsigned literal_length{-1u};
    unsigned offset{-1u};
    unsigned match_length{-1u};
};

std::basic_string<uint8_t> decompress(const std::basic_string<uint8_t> &data) {
    CHECK(!data.empty());
    unsigned data_len_bits = data.size() * 8;
    CHECK(data.back() != 0);
    for (uint8_t tail = 0; tail < 8; ++tail) {
        if ((data.back() >> tail) & 1) {
            data_len_bits -= tail + 1;
            break;
        }
    }

    td::ConstBitPtr data_bits(data.data(), 0);
    const auto get_uint = [&](unsigned bits) {
        const unsigned res = data_bits.get_uint(bits);
        data_bits.offs += static_cast<int>(bits);
        return res;
    };

    // number of bits to store offset size
    uint8_t bits_for_offset = get_uint(4);
    // minimal offset size. e.g. x in "bits for offset" field means the offset takes "x + min_bits_for_offset" bits
    uint8_t min_bits_for_offset = get_uint(4);

    std::vector<Token> tokens;
    unsigned sum_literals_length = 0;
    while (1) {
        Token token;
        uint8_t byte = get_uint(8);

        unsigned literal_length = byte >> MATCH_NIBBLE_BITS;
        if (literal_length == (1u << LITERAL_NIBBLE_BITS) - 1) {
            while (true) {
                const uint8_t cur = get_uint(8);
                literal_length += cur;
                if (cur < 255) {
                    break;
                }
            }
        }
        sum_literals_length += literal_length;
        token.literal_length = literal_length;

        // read match length
        unsigned match_length = byte & ((1u << MATCH_NIBBLE_BITS) - 1);
        if (match_length == (1u << MATCH_NIBBLE_BITS) - 1) {
            while (true) {
                const uint8_t cur = get_uint(8);
                match_length += cur;
                if (cur < 255) {
                    break;
                }
            }
        }
        token.match_length = match_length;

        // read offset
        if (sum_literals_length * 8 + data_bits.offs >= data_len_bits) {
            CHECK(sum_literals_length * 8 + data_bits.offs == data_len_bits);
            CHECK(match_length == 0);
            tokens.push_back(token);
            break;
        }
        token.match_length += MATCH_OFFSET;

        unsigned offset_bit_length = get_uint(bits_for_offset);
        offset_bit_length += min_bits_for_offset;
        unsigned offset = get_uint(offset_bit_length);
        token.offset = offset;

        tokens.push_back(token);
    }

    std::basic_string<uint8_t> out;
    for (const auto& token : tokens) {
        // read literal
        CHECK(token.literal_length != -1u);
        for (unsigned it = 0; it < token.literal_length; ++it) {
            out += static_cast<uint8_t>(get_uint(8));
        }

        // read match
        if (token.offset != -1u) {
            const unsigned start_index = out.size() - token.offset;
            for (unsigned it = 0; it < token.match_length; ++it) {
                const uint8_t ch = out[start_index + it];
                out += ch;
            }
        }
    }

    return out;
}
};
