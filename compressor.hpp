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

    // sort cyclic shifts
    p = build_suff_arr(data);

    pos.resize(n);
    for (unsigned i = 0; i < n; ++i) {
        pos[p[i]] = i;
    }
    const auto lcp = get_lcp_from_suff_arr(data, p, pos);
    const auto get_lcp_loop = [&](unsigned j, unsigned i) -> unsigned {
        j = p[j];
        i = p[i];
        unsigned len = 0;
        while (i + len < n && j + len < n && data[i + len] == data[j + len]) {
            ++len;
        }
        return len;
    };
    SparseTableMin sparse_table(lcp);
    const auto get_lcp_fast = [&](unsigned j, unsigned i) -> unsigned {
        if (i > j) {
            std::swap(i, j);
        }
        const auto res = sparse_table.get_min(i, j);
        return res;
    };

    std::vector<unsigned> best_len(n + 1, 0);
    std::vector<unsigned> best_offset(n, -1u);
    std::set<unsigned> alive;
    const unsigned ITERS = n > (1 << 16) ? 10 : 50;
    for (unsigned i = 0; i < n; ++i) {
        if (i > 0) {
            alive.insert(pos[i - 1]);
        }
        if (i < dict.size()) {
            continue;
        }

        const unsigned pos_i = pos[i];
        // to the left and to the right in suff arr
        for (unsigned phase = 0; phase < 2; ++phase) {
            auto it = alive.upper_bound(pos[i]);
            for (unsigned iter = 0; iter < ITERS; ++iter) {
                if (phase == 1) {
                    if (it != alive.begin()) {
                        --it;
                    } else {
                        break;
                    }
                }
                if (it == alive.end()) {
                    break;
                }
                const auto len = get_lcp_fast(*it, pos_i);
                if (len >= MATCH_LENGHT_OFFSET) {
                    const unsigned j = p[*it];
                    CHECK(j < i);
                    const unsigned offset = i - j;
                    if (len > best_len[i] || (len == best_len[i] && offset < best_offset[i])) {
                        best_offset[i] = offset;
                        best_len[i] = len;
                    }
                }
                if (phase == 0) {
                    ++it;
                }
            }
        }
    }

    const auto push_offset = [&](unsigned offset) -> unsigned {
        const unsigned start_offs = out.offs;
        const uint8_t offset_bit_length = std::max(min_bits_for_offset, len_in_bits(offset));
        store_uint(offset_bit_length - min_bits_for_offset, bits_for_offset);
        store_uint(offset, offset_bit_length);
        return out.offs - start_offs;
    };

    //  encoding
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
    auto dump_lengths = [&](unsigned match_len, unsigned literal_len) -> unsigned {
        const unsigned start_offs = out.offs;

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

        return out.offs - start_offs;
    };
    unsigned sum_matches_len = 0;
    unsigned bits_spent_on_tokens = 0;
    std::basic_string<uint8_t> literals;
    std::set<unsigned> suff_referenced;
    unsigned cnt_references = 0;
    for (unsigned i = dict.size(); i < n;) {
        unsigned j = i;
        while (j < n && best_len[j] < MATCH_LENGHT_OFFSET) {
            ++j;
        }
        const auto literal_len = j - i;
        const auto match_len = best_len[j];
        sum_matches_len += match_len;
        literals += data.substr(i, literal_len);
        bits_spent_on_tokens += dump_lengths(match_len, literal_len);
        if (j < n) {
            bits_spent_on_tokens += push_offset(best_offset[j]);
            suff_referenced.insert(j - best_offset[j]);
            cnt_references += 1;
        }
        i = j + best_len[j];
    }

    std::cout << "serialize in bytes: " << " sum_match_len(bytes)=" << sum_matches_len
            << ", bits_spent_on_tokens=" << bits_spent_on_tokens << ", cnt diff suff referenced: "
            << suff_referenced.size() << ", cnt_references: " << cnt_references
    << ", overall " << data.size() * 8 << " bits" << std::endl;

    while (out.offs % 8 != 0) {
        store_uint(0, 1);
    }

    for (const auto &byte: literals) {
        store_uint(byte, 8);
    }
    CHECK(out.offs % 8 == 0);
    const unsigned len_in_bytes = out.offs / 8;
    return {out.ptr, len_in_bytes};
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
