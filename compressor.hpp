#pragma once

#include <numeric>
#include <set>
#include <string>
#include <td/utils/check.h>

// #include "utils.h"

namespace LZ_compressor {
std::vector<unsigned> build_suff_arr(const std::basic_string<uint8_t> &s) {
    const unsigned n = s.size();
    std::vector<unsigned> p(n);

    const unsigned C = std::max(256u, n);

    std::vector<unsigned> cnt(C, 0);
    for (uint8_t ch: s) {
        ++cnt[ch];
    }
    for (unsigned i = 0; i + 1 < 256; ++i) {
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
            // std::cout << "c[j2] = " << c[j] << std::endl;
            // std::cout << "cnt[c[j2]] = " << cnt[c[j2]] << std::endl;
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

static const uint8_t MAX_BYTES_FOR_INDEX = 2;

std::basic_string<uint8_t> compress_standard(const std::basic_string<uint8_t> &data) {
    auto res = td::lz4_compress(td::BufferSlice(reinterpret_cast<const char *>(data.data()), data.size()));
    return {reinterpret_cast<const uint8_t *>(res.data()), res.size()};
}

std::basic_string<uint8_t> compress(const std::basic_string<uint8_t> &data) {
    // sort cyclic shifts
    const std::vector<unsigned> p = build_suff_arr(data);

    const unsigned n = p.size();
    CHECK(n == data.size());

    std::vector<unsigned> pos(n);
    for (unsigned i = 0; i < n; ++i) {
        pos[p[i]] = i;
    }

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

    std::basic_string<uint8_t> out;
    std::set<unsigned> alive_pos;
    std::basic_string<uint8_t> literal_buf;
    auto dump_literal = [&]() -> void {
        if (literal_buf.empty()) {
            return;
        }
        unsigned len = literal_buf.size();
        for (unsigned it = 0; ; ++it) {
            const unsigned limit = (it == 0) ? 127 : 255;
            const unsigned cur = std::min(len, limit);
            const unsigned cur_byte = (it == 0) ? (cur * 2 + 1) : cur;
            CHECK(cur_byte <= 255);
            out.push_back(cur_byte);
            len -= cur;
            if (cur < limit) {
                break;
            }
        }
        CHECK(len == 0);
        out += literal_buf;
        literal_buf.clear();
    };
    auto bytes_for_len = [&](unsigned len) -> unsigned {
        CHECK(len <= n);
        unsigned cnt = 0;
        while (true) {
            const unsigned limit = (cnt == 0) ? 127 : 255;
            const unsigned cur = std::min(limit, len);
            len -= cur;
            ++cnt;
            if (cur < limit) {
                break;
            }
        }
        return cnt;
    };
    unsigned leftmost_alive = 0;
    uint8_t bytes_for_index = 1;
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
        unsigned best_prev = -1u;
        unsigned best_len = 0;
        // to the left and to the right in suff arr
        for (unsigned phase = 0; phase < 2; ++phase) {
            std::set<unsigned>::const_iterator it;
            if (phase == 0) {
                it = alive_pos.lower_bound(pos[i]);
            } else {
                it = alive_pos.upper_bound(pos[i]);
            }
            if (it != alive_pos.end()) {
                unsigned j = p[*it];
                CHECK(j < i);
                const auto len = get_lcp(j, i);
                if (len > best_len) {
                    best_prev = j;
                    best_len = len;
                }
            }
        }

        // encoding with link should be better than encoding with literal
        unsigned with_literal = best_len + (literal_buf.empty() ? 1 : 0);
        unsigned with_link = bytes_for_len(best_len) + bytes_for_index;
        if (with_link < with_literal) {
            CHECK(best_len > 0);
            dump_literal();

            // dump length of the match
            unsigned len = best_len;
            for (unsigned it = 0; ; ++it) {
                const unsigned limit = (it == 0) ? 127 : 255;
                const unsigned cur = std::min(limit, len);
                const unsigned cur_byte = (it == 0) ? (cur * 2 + 0) : cur;
                out.push_back(cur_byte);
                len -= cur;
                if (cur < limit) {
                    break;
                }
            }
            CHECK(len == 0);

            // dump match position
            unsigned j = best_prev;
            CHECK(j < i);
            CHECK(len_in_bytes(i - j) <= MAX_BYTES_FOR_INDEX);
            push_as_bytes(out, i - j, bytes_for_index);
            CHECK(best_len > 0);
            for (unsigned it = 0; it < best_len; ++it) {
                alive_pos.insert(pos[i++]);
            }
        } else {
            literal_buf.push_back(data[i]);
            alive_pos.insert(pos[i]);
            ++i;
        }
    }
    dump_literal();
    return out;
}

std::basic_string<uint8_t> decompress_standard(const std::basic_string<uint8_t> &data) {
    auto res = td::lz4_decompress(
        td::BufferSlice(reinterpret_cast<const char *>(data.data()), data.size()),
        2 << 20
    ).move_as_ok();
    return {reinterpret_cast<const uint8_t *>(res.data()), res.size()};
}

std::basic_string<uint8_t> decompress(const std::basic_string<uint8_t> &data) {
    std::basic_string<uint8_t> out;
    uint8_t bytes_for_index = 1;
    for (unsigned i = 0; i < data.size();) {
        if (out.size() >= (1u << (8 * bytes_for_index)) && bytes_for_index < MAX_BYTES_FOR_INDEX) {
            ++bytes_for_index;
        }

        uint8_t token = data[i];
        if (token & 1) {
            // literal
            unsigned len = 0;
            for (unsigned it = 0; ; ++it) {
                const unsigned limit = (it == 0) ? 127 : 255;
                uint8_t byte = data[i++];
                const unsigned cur = (it == 0) ? (byte >> 1) : (byte);
                len += cur;
                CHECK(cur <= limit);
                if (cur < limit) {
                    break;
                }
            }
            for (unsigned it = 0; it < len; ++it) {
                out.push_back(data[i++]);
            }
        } else {
            // match with earlier suffix

            // decode match length
            unsigned len = 0;
            for (unsigned it = 0; ; ++it) {
                const unsigned limit = (it == 0) ? 127 : 255;
                uint8_t byte = data[i++];
                const unsigned cur = (it == 0) ? (byte >> 1) : (byte);
                len += cur;
                CHECK(cur <= limit);
                if (cur < limit) {
                    break;
                }
            }

            // decode match offset
            unsigned offset = 0;
            for (unsigned it = 0; it < bytes_for_index; ++it) {
                offset = (offset << 8) ^ data[i++];
            }
            CHECK(out.size() >= offset);
            unsigned j = out.size() - offset;
            for (unsigned it = 0; it < len; ++it) {
                const auto byte = out[j];
                out.push_back(byte);
                ++j;
            }
        }
    }
    return out;
}
};
