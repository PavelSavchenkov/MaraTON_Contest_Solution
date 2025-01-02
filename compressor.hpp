#pragma once

#include <numeric>
#include <set>
#include <string>
#include <td/utils/check.h>

namespace LZ_compressor {
static const uint8_t MAX_BYTES_FOR_INDEX = 2;
static const unsigned MIN_MATCH_LENGTH = 4;

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

    std::basic_string<uint8_t> out;
    std::set<unsigned> alive_pos;
    std::basic_string<uint8_t> literal_buf;
    auto dump_int_number_per_255 = [&](unsigned number) -> void {
        while (1) {
            unsigned cur = std::min(number, 255u);
            number -= cur;
            out.push_back(cur);
            if (cur < 255) {
                break;
            }
        }
    };
    auto dump_tokens_and_literal = [&](unsigned match_length) {
        unsigned literal_length = literal_buf.size();
        unsigned literal_nibble = std::min(literal_length, 15u);
        literal_length -= literal_nibble;

        uint8_t token = 0;
        token = literal_nibble << 4;

        CHECK(match_length >= MIN_MATCH_LENGTH);
        if (match_length == -1u) {
            match_length = 0;
        } else {
            match_length -= MIN_MATCH_LENGTH;
        }
        unsigned match_nibble = std::min(match_length, 15u);
        match_length -= match_nibble;
        token ^= match_nibble;

        out.push_back(token);

        if (literal_nibble == 15) {
            dump_int_number_per_255(literal_length);
        }

        out += literal_buf;
        literal_buf.clear();

        if (match_nibble == 15) {
            dump_int_number_per_255(match_length);
        }
    };
    unsigned leftmost_alive = 0;
    uint8_t bytes_for_index = 1;
    std::map<uint64_t, unsigned> map_pref;
    auto make_alive = [&](unsigned i) {
        alive_pos.insert(pos[i]);
        if (i + MIN_MATCH_LENGTH <= n) {
            uint64_t key = 0;
            for (unsigned j = 0; j < MIN_MATCH_LENGTH; ++j) {
                key = (key << 8) ^ data[i + j];
            }
            map_pref[key] = i;
        }
    };
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

        // check the last occurence of suff
        // OVERWRITE suff arr search
        {
            best_len = 0;
            best_prev = -1u;
            if (i + MIN_MATCH_LENGTH <= n) {
                uint64_t key = 0;
                for (unsigned j = 0; j < MIN_MATCH_LENGTH; ++j) {
                    key = (key << 8) ^ data[i + j];
                }
                if (map_pref.count(key)) {
                    best_prev = map_pref[key];
                    if (len_in_bytes(i - best_prev) <= MAX_BYTES_FOR_INDEX) {
                        best_len = get_lcp(best_prev, i);
                    }
                }
            }
        }

        if (best_len >= MIN_MATCH_LENGTH) {
            CHECK(best_len > 0);

            dump_tokens_and_literal(best_len);

            // dump match offset
            unsigned j = best_prev;
            CHECK(j < i);
            CHECK(len_in_bytes(i - j) <= MAX_BYTES_FOR_INDEX);
            push_as_bytes(out, i - j, bytes_for_index);
            for (unsigned it = 0; it < best_len; ++it) {
                make_alive(i);
                ++i;
            }
        } else {
            literal_buf.push_back(data[i]);
            make_alive(i);
            ++i;
        }
    }
    if (!literal_buf.empty()) {
        dump_tokens_and_literal(-1u);
    }
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
        uint8_t token = data[i++];

        unsigned literal_length = token >> 4;
        if (literal_length == 15) {
            while (1) {
                literal_length += data[i];
                if (data[i++] < 255) {
                    break;
                }
            }
        }
        // read literals
        while (literal_length > 0) {
            --literal_length;
            CHECK(i < data.size());
            out += data[i++];
        }
        // read match length
        unsigned match_length = token & 0x0F;
        if (match_length == 15) {
            while (1) {
                match_length += data[i];
                if (data[i++] < 255) {
                    break;
                }
            }
        }
        // read offset
        if (i == data.size()) {
            CHECK(match_length == 0);
            break;
        }
        match_length += MIN_MATCH_LENGTH;

        if (out.size() >= (1u << (8 * bytes_for_index)) && bytes_for_index < MAX_BYTES_FOR_INDEX) {
            ++bytes_for_index;
        }

        unsigned offset = 0;
        for (unsigned it = 0; it < bytes_for_index; ++it) {
            offset = (offset << 8) ^ data[i++];
        }
        CHECK(out.size() >= offset);
        unsigned j = out.size() - offset;
        for (unsigned it = 0; it < match_length; ++it) {
            const auto byte = out[j];
            out.push_back(byte);
            ++j;
        }
    }
    return out;
}
};
