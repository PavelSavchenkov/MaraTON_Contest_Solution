#pragma once

#include <numeric>
#include <set>
#include <string>
#include <td/utils/check.h>

#include "utils.h"

namespace LZ_compressor {
static const unsigned MATCH_LENGHT_OFFSET = 4;
static const uint8_t LITERAL_NIBBLE_BITS = 6;
static const uint8_t MATCH_NIBBLE_BITS = 8 - LITERAL_NIBBLE_BITS;

static const unsigned MAX_LOGN = 21;
static const unsigned MAXN = 1 << MAX_LOGN;

namespace SparseTableMin {
static unsigned min[MAX_LOGN + 1][MAXN];

void build(const std::vector<unsigned> &values) {
    const unsigned n = values.size();
    const unsigned logn = len_in_bits(n);
    memset(min, -1, sizeof min);

    for (unsigned i = 0; i < n; ++i) {
        min[0][i] = values[i];
    }
    for (unsigned pw = 1; pw <= logn; ++pw) {
        for (unsigned i = 0; i < n; ++i) {
            min[pw][i] = std::min(
                min[pw - 1][i],
                min[pw - 1][std::min(n - 1, i + (1u << (pw - 1)))]
            );
        }
    }
}

// [l, r)
unsigned get_min(const unsigned l, const unsigned r) {
    CHECK(l < r);
    const unsigned pw = len_in_bits(r - l) - 1;
    return std::min(min[pw][l], min[pw][r - (1u << pw)]);
}
};

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

std::basic_string<uint8_t> compress(
    const std::basic_string<uint8_t> &data_,
    const std::basic_string<uint8_t> &dict
) {
    CHECK(!data_.empty());

    const auto data = dict + data_;

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
    // calculate above constants
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

    // -----------------------------------------------
    // build suff arr and precalc stuff for further dp
    std::vector<unsigned> p{};
    std::vector<unsigned> pos{};
    const unsigned n = data.size();
    const unsigned logn = len_in_bits(n);

    // sort cyclic shifts
    p = build_suff_arr(data);

    pos.resize(n);
    for (unsigned i = 0; i < n; ++i) {
        pos[p[i]] = i;
    }

    // lcp[i] is lcp of p[i] and p[i + 1]
    std::vector<unsigned> lcp(n, 0);
    unsigned lcp_len = 0;
    for (unsigned i = 0; i < n; ++i) {
        const unsigned pi = pos[i];
        if (pi == n - 1) {
            continue;
        }
        const unsigned j = p[pi + 1];
        lcp_len = std::min(lcp_len, n - j);
        while (i + lcp_len < n && j + lcp_len < n && data[i + lcp_len] == data[j + lcp_len]) {
            ++lcp_len;
        }
        lcp[pi] = lcp_len;
        if (lcp_len > 0) {
            --lcp_len;
        }
    }
    SparseTableMin::build(lcp);
    const auto get_lcp_fast = [&](unsigned j, unsigned i) -> unsigned {
        if (i > j) {
            std::swap(i, j);
        }
        const auto res = SparseTableMin::get_min(i, j);
        return res;
    };

    Timer* timer = new Timer("best_suff and best_len");
    static unsigned best_len[MAX_LOGN + 1][MAXN];
    static unsigned best_suff[MAX_LOGN + 1][MAXN];
    memset (best_len, 0, sizeof (best_len));
    memset (best_suff, -1, sizeof (best_suff));
    static std::set<unsigned> alive_on_dist[MAX_LOGN + 1];
    for (unsigned i = 0; i <= MAX_LOGN; ++i) {
        alive_on_dist[i].clear();
    }
    for (unsigned i = 0; i < n; ++i) {
        // rearrange prev indexes
        for (unsigned hb = 1; hb <= logn; ++hb) {
            const unsigned offset = 1u << hb;
            // i - offset >= 0
            if (i >= offset) {
                const unsigned j = i - offset;
                const unsigned pj = pos[j];
                // CHECK(alive_on_dist[hb - 1].count(pj));
                alive_on_dist[hb - 1].erase(pj);
                alive_on_dist[hb].insert(pj);
            } else {
                break;
            }
        }
        // insert i-1
        if (i > 0) {
            alive_on_dist[0].insert(pos[i - 1]);
        }

        const unsigned pos_i = pos[i];
        for (unsigned hb = 0; hb <= logn; ++hb) {
            const auto &S = alive_on_dist[hb];
            if (S.empty()) {
                continue;
            }

            // to the left and to the right in suff arr
            for (unsigned phase = 0; phase < 2; ++phase) {
                auto it = S.upper_bound(pos[i]);
                if (phase == 1) {
                    if (it != S.begin()) {
                        --it;
                    }
                }
                if (it == S.end()) {
                    continue;
                }
                const auto len = get_lcp_fast(*it, pos_i);
                if (len >= MATCH_LENGHT_OFFSET && len > best_len[hb][i]) {
                    const unsigned j = p[*it];
                    CHECK(j < i);
                    const unsigned offset = i - j;
                    // CHECK(len_in_bits(offset) - 1 == hb);
                    best_suff[hb][i] = j;
                    best_len[hb][i] = len;
                }
            }
        }
    }
    delete timer;

    // calculate dp and then encode
    const auto how_many_bits_to_encode_match = [&](unsigned offset, unsigned len) {
        unsigned bits = MATCH_NIBBLE_BITS; // in token
        CHECK(len != -1u);
        if (len == 0) {
            return bits;
        }
        CHECK(len >= MATCH_LENGHT_OFFSET);
        len -= MATCH_LENGHT_OFFSET;
        if (len >= ((1u << MATCH_NIBBLE_BITS) - 1)) {
            const unsigned rest = len - ((1u << MATCH_NIBBLE_BITS) - 1);
            CHECK(rest <= n);
            bits += (rest + 255) / 255 * 8;
        }
        bits += bits_for_offset;
        bits += std::max(min_bits_for_offset, len_in_bits(offset));
        return bits;
    };
    const auto how_many_bits_to_encode_literal = [&](unsigned len) {
        unsigned bits = LITERAL_NIBBLE_BITS;
        if (len >= ((1u << LITERAL_NIBBLE_BITS) - 1)) {
            const unsigned rest = len - ((1u << LITERAL_NIBBLE_BITS) - 1);
            CHECK(rest <= n);
            bits += (rest + 255) / 255 * 8;
        }
        bits += len * 8;
        return bits;
    };
    const auto how_many_bits_to_encode = [&](unsigned offset, unsigned match_length, unsigned literal_length) {
        return how_many_bits_to_encode_literal(literal_length) +
               how_many_bits_to_encode_match(offset, match_length);
    };
    const auto push_offset = [&](unsigned offset) {
        const uint8_t offset_bit_length = std::max(min_bits_for_offset, len_in_bits(offset));
        store_uint(offset_bit_length - min_bits_for_offset, bits_for_offset);
        store_uint(offset, offset_bit_length);
    };

    // calculate dp[i] = minimum number of bits to encode prefix of length i, for i >= dict.size()
    std::vector<unsigned> next_with_match(n, n);
    for (unsigned it = 0; it < n; ++it) {
        const unsigned i = n - it - 1;
        if (i + 1 < n) {
            next_with_match[i] = next_with_match[i + 1];
        }
        for (unsigned j = 0; j <= logn; ++j) {
            if (best_len[j][i] >= MATCH_LENGHT_OFFSET) {
                next_with_match[i] = i;
                break;
            }
        }
        if (i == dict.size()) {
            break;
        }
    }
    std::vector<unsigned> dp(n + 1, -1u);
    std::vector<unsigned> best_literal_len(n + 1, -1u);
    std::vector<unsigned> best_offset(n + 1, -1u);
    std::vector<unsigned> best_i(n + 1, -1u);
    dp[dict.size()] = 0;
    const auto relax_dp = [&](unsigned j, unsigned i, unsigned literal_len, unsigned offset) {
        const unsigned len = j - i;
        CHECK(literal_len <= len);
        CHECK(len <= n);
        unsigned match_len = len - literal_len;
        const unsigned new_dp = dp[i] + how_many_bits_to_encode(offset, match_len, literal_len);
        if (new_dp < dp[j]) {
            dp[j] = new_dp;
            best_literal_len[j] = literal_len;
            best_offset[j] = offset;
            best_i[j] = i;
        }
    };
    for (unsigned i = dict.size(); i < n; ++i) {
        if (dp[i] == -1u) {
            continue;
        }
        const unsigned j = next_with_match[i];
        relax_dp(j, i, j - i, -1u);
        for (unsigned hb = 0; hb <= logn; ++hb) {
            if (best_len[hb][j] == -1u) {
                continue;
            }
            const unsigned offset = j - best_suff[hb][j];
            relax_dp(j + best_len[hb][j], i, j - i, offset);
        }
    }

    // -------------------------
    // actual encoding
    auto dump_int_number_per_255 = [&](unsigned number) -> void {
        while (true) {
            unsigned cur = std::min(number, 255u);
            number -= cur;
            store_uint(cur, 8);
            if (cur < 255) {
                CHECK(number == 0);
                break;
            }
        }
    };
    auto dump_lengths = [&](unsigned match_len, unsigned literal_len) {
        unsigned literal_nibble = std::min(literal_len, (1u << LITERAL_NIBBLE_BITS) - 1);
        literal_len -= literal_nibble;

        uint8_t token = 0;
        token = literal_nibble << MATCH_NIBBLE_BITS;

        CHECK(match_len != -1u);
        if (match_len != 0) {
            match_len -= MATCH_LENGHT_OFFSET;
        }
        unsigned match_nibble = std::min(match_len, (1u << MATCH_NIBBLE_BITS) - 1);
        match_len -= match_nibble;
        token ^= match_nibble;

        store_uint(token, 8);

        if (literal_nibble == (1u << LITERAL_NIBBLE_BITS) - 1) {
            dump_int_number_per_255(literal_len);
        }

        if (match_nibble == (1u << MATCH_NIBBLE_BITS) - 1) {
            dump_int_number_per_255(match_len);
        }
    };

    std::vector<unsigned> js;
    // stack
    {
        unsigned j = n;
        CHECK(dp[j] <= n * 8);
        while (j != dict.size()) {
            js.push_back(j);
            CHECK(dp[j] <= n * 8);
            j = best_i[j];
        }
        js.push_back(dict.size());
        std::reverse(js.begin(), js.end());
    }

    std::basic_string<uint8_t> all_literals;
    for (unsigned it = 0; it + 1 < js.size(); ++it) {
        const unsigned i = js[it];
        const unsigned nxt = js[it + 1];
        const unsigned len = nxt - i;
        const unsigned literal_len = best_literal_len[nxt];
        const unsigned match_len = (len - literal_len);
        const unsigned offset = best_offset[nxt];
        all_literals += data.substr(i, literal_len);
        dump_lengths(match_len, literal_len);
        if (match_len > 0) {
            push_offset(offset);
        } else {
            CHECK(it + 2 == js.size());
        }
    }

    while (out.offs % 8 != 0) {
        store_uint(0, 1);
    }

    for (const auto &byte: all_literals) {
        store_uint(byte, 8);
    }
    CHECK(out.offs % 8 == 0);
    const unsigned len_in_bytes = out.offs / 8;
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

std::basic_string<uint8_t> decompress(
    const std::basic_string<uint8_t> &data,
    const std::basic_string<uint8_t> &dict = {}
) {
    if (data.empty()) {
        return {};
    }

    const unsigned data_len_bytes = data.size();
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
    while (true) {
        const uint8_t byte = get_uint(8);
        Token token;

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

        unsigned bytes_processed = (data_bits.offs + 7) / 8;
        if (sum_literals_length + bytes_processed >= data_len_bytes) {
            CHECK(match_length == 0);
            CHECK(sum_literals_length + bytes_processed == data_len_bytes);
            tokens.push_back(token);
            break;
        }
        token.match_length += MATCH_LENGHT_OFFSET;

        // read offset
        unsigned offset_bit_length = get_uint(bits_for_offset);
        offset_bit_length += min_bits_for_offset;
        unsigned offset = get_uint(offset_bit_length);
        token.offset = offset;

        tokens.push_back(token);

        bytes_processed = (data_bits.offs + 7) / 8;
        if (sum_literals_length + bytes_processed >= data_len_bytes) {
            CHECK(sum_literals_length + bytes_processed == data_len_bytes);
            break;
        }
    }

    while (data_bits.offs % 8 != 0) {
        ++data_bits.offs;
    }

    std::basic_string<uint8_t> out = dict;
    for (const auto &token: tokens) {
        // read literal
        CHECK(token.literal_length != -1u);
        for (unsigned it = 0; it < token.literal_length; ++it) {
            out += static_cast<uint8_t>(get_uint(8));
        }

        // read match
        if (token.offset != -1u) {
            CHECK(token.offset <= out.size());
            const unsigned start_index = out.size() - token.offset;
            for (unsigned it = 0; it < token.match_length; ++it) {
                const uint8_t ch = out[start_index + it];
                out += ch;
            }
        }
    }

    return out.substr(dict.size());
}
};
