#pragma once
#include "utils.h"

namespace lz_compressor_bits {
static const unsigned MIN_MATCH_LENGTH = 4 * 8;
static const unsigned LITERAL_LENGTH_BITS_BLOCK = 9;
static const unsigned MATCH_LENGTH_BITS_BLOCK = 8;

std::basic_string<uint8_t> compress(const std::basic_string<uint8_t> &input_bytes) {
    std::basic_string<uint8_t> input_bits(input_bytes.size() * 8, 0);
    for (unsigned i = 0; i < input_bits.size(); ++i) {
        input_bits[i] = ((input_bytes[i / 8] >> (7 - i % 8)) & 1);
    }
    td::ConstBitPtr input_bits_ptr(input_bytes.data(), 0);
    for (unsigned i = 0; i < input_bytes.size() * 8; ++i) {
        bool bit = input_bits_ptr.get_uint(1);
        input_bits_ptr.offs += 1;
        CHECK(bit == input_bits[i]);
    }

    // check for potential stop symbol
    {
        auto stop_symbol = get_min_bit_str_not_in(input_bits);
        std::cout << "stop symbol bit string, len= " << int(len_in_bits(stop_symbol)) << ", symbol=";
        for (unsigned i = len_in_bits(stop_symbol); i > 0; --i) {
            std::cout << ((stop_symbol >> (i - 1)) & 1);
        }
        std::cout << std::endl;
    }

    const unsigned n = input_bits.size();
    std::vector<unsigned> p; {
        Timer timer("suff_arr");
        p = build_suff_arr(input_bits);
    }

    std::vector<unsigned> pos(n);
    for (unsigned i = 0; i < n; ++i) {
        pos[p[i]] = i;
    }
    std::vector<unsigned> lcp; {
        Timer timer("lcp");
        lcp = get_lcp_from_suff_arr(
            input_bits,
            p,
            pos
        );
    }

    SparseTableMin table_min; {
        Timer timer("table_min");
        table_min = SparseTableMin(lcp);
    }

    const auto get_lcp_pos = [&](unsigned pi, unsigned pj) {
        if (pi > pj) {
            std::swap(pi, pj);
        }
        return table_min.get_min(pi, pj);
    };

    // TODO: add segm tree max
    // TODO: add treap

    std::vector<unsigned> best_match_suff(n, -1u);
    std::vector<unsigned> best_match_len(n, 0);
    std::set<unsigned> alive_pos; {
        Timer timer("Build best matches");
        for (unsigned i = 0; i < n; ++i) {
            // static const unsigned bound = 1u << 20;
            // if (i >= bound) {
            //     alive_pos.erase(pos[i - bound]);
            // }
            const auto pos_i = pos[i];
            auto it = alive_pos.upper_bound(pos_i);
            for (unsigned phase = 0; phase < 2; ++phase) {
                if (phase == 1) {
                    if (it != alive_pos.begin()) {
                        --it;
                    } else {
                        continue;
                    }
                }
                if (it == alive_pos.end()) {
                    continue;
                }
                const auto pos_j = *it;
                const auto match_len = get_lcp_pos(pos_i, pos_j);
                if (match_len < MIN_MATCH_LENGTH) {
                    continue;
                }
                if (match_len > best_match_len[i]) {
                    best_match_len[i] = match_len;
                    const auto j = p[pos_j];
                    CHECK(j < i);
                    best_match_suff[i] = j;
                }
            }
            alive_pos.insert(pos_i);
        }
    }

    std::basic_string<uint8_t> output_bytes(n / 8 + 1, 0);
    td::BitPtr output_bits(output_bytes.data(), 0);

    const auto push_number_as_blocks = [&](unsigned num, const uint8_t block_len) -> unsigned {
        const unsigned block_num = (1u << block_len) - 1;
        unsigned bits_spent = 0;
        while (true) {
            const unsigned cur = std::min(block_num, num);
            output_bits.store_uint(cur, block_len);
            output_bits.offs += block_len;
            bits_spent += block_len;
            num -= cur;
            if (cur < block_num) {
                break;
            }
        }
        return bits_spent;
    };

    // Push serialization bits
    {
        Timer timer("Push serialization bits");
        unsigned sum_length_matches = 0;
        unsigned bits_spent_on_literal_tokens = 0;
        unsigned bits_spent_on_match_len_tokens = 0;
        unsigned bits_spent_on_match_offset_tokens = 0;
        unsigned last_literal_start = 0;
        std::set<unsigned> suff_referenced;
        unsigned sum_literal_lengths = 0;
        unsigned cnt_references = 0;
        std::vector<unsigned> match_lengths{};
        for (unsigned i = 0; i < n;) {
            const auto match_len = best_match_len[i];
            if (match_len >= MIN_MATCH_LENGTH) {
                // naive check of the match
                {
                    unsigned from = best_match_suff[i];
                    unsigned to = i;
                    for (unsigned it = 0; it < match_len; ++it) {
                        CHECK(input_bits[from + it] == input_bits[to + it]);
                    }
                }
                sum_length_matches += match_len;
                suff_referenced.insert(best_match_suff[i]);
                cnt_references += 1;

                CHECK(last_literal_start <= i);
                const auto literal_len = i - last_literal_start;
                sum_literal_lengths += literal_len;
                bits_spent_on_literal_tokens += push_number_as_blocks(literal_len, LITERAL_LENGTH_BITS_BLOCK);
                for (unsigned it = 0; it < literal_len; ++it) {
                    output_bits.store_uint(input_bits[last_literal_start + it], 1);
                    output_bits.offs += 1;
                }
                bits_spent_on_match_len_tokens += push_number_as_blocks(match_len, MATCH_LENGTH_BITS_BLOCK);
                const unsigned offset = i - best_match_suff[i];
                std::cout << "offset=" << offset << ", match_len=" << match_len << std::endl;
                match_lengths.push_back(match_len);
                const unsigned bit_len = len_in_bits(i);
                bits_spent_on_match_offset_tokens += bit_len;
                output_bits.store_uint(offset, bit_len);
                output_bits.offs += bit_len;

                i += match_len;
                last_literal_start = i;
            } else {
                ++i;
            }
        }
        if (last_literal_start < n) {
            const auto literal_len = n - last_literal_start;
            bits_spent_on_literal_tokens += push_number_as_blocks(literal_len, LITERAL_LENGTH_BITS_BLOCK);
            for (unsigned it = 0; it < literal_len; ++it) {
                output_bits.store_uint(input_bits[last_literal_start + it], 1);
                output_bits.offs += 1;
            }
        }
        output_bits.store_uint(1, 1);
        output_bits.offs += 1;

        const unsigned bits_spent_on_tokens = bits_spent_on_literal_tokens + bits_spent_on_match_len_tokens +
                                              bits_spent_on_match_offset_tokens;
        std::sort(match_lengths.begin(), match_lengths.end());
        const unsigned match_len_p90 = match_lengths[match_lengths.size() * 9 / 10];
        std::cout << "serialize: " << "bits_spent_on_tokens=" << bits_spent_on_tokens
                << "\nsum_matches_len=" << sum_length_matches
                << "\nsum_matches_len in bytes: " << sum_length_matches / 8
                << "\ncnt diff suff referenced: " << suff_referenced.size()
                << "\ncnt_references: " << cnt_references
                << "\nbits_spent_on_literal_tokens: " << bits_spent_on_literal_tokens
                << "\nbits_spent_on_match_len_tokens: " << bits_spent_on_match_len_tokens
                << "\nbits_spent_on_match_offset_tokens: " << bits_spent_on_match_offset_tokens
                << "\navg literal len: " << sum_literal_lengths * 1.0 / cnt_references
                << "\nmatch_len_p90: " << match_len_p90
                << "\noverall " << input_bits.size() << " bits" << std::endl;
    }

    return {output_bits.ptr, static_cast<size_t>(output_bits.offs + 7) / 8};
}

std::basic_string<uint8_t> decompress(const std::basic_string<uint8_t> &data) {
    td::ConstBitPtr data_bits(data.data(), 0);

    unsigned n_bits = data.size() * 8;
    CHECK(data.back() != 0);
    for (unsigned i = 0; i < 8; ++i) {
        if ((data.back() >> i) & 1) {
            n_bits -= (i + 1);
            break;
        }
    }

    const auto get_number_as_blocks = [&](const uint8_t block_len) {
        const unsigned block_num = (1u << block_len) - 1;
        unsigned num = 0;
        while (true) {
            const unsigned cur = data_bits.get_uint(block_len);
            data_bits.offs += block_len;
            num += cur;
            if (cur < block_num) {
                break;
            }
        }
        return num;
    };

    std::basic_string<uint8_t> output_bytes(2 << 20, 0);
    td::BitPtr output_bits(output_bytes.data(), 0);
    while (data_bits.offs < n_bits) {
        const auto literal_len = get_number_as_blocks(LITERAL_LENGTH_BITS_BLOCK);

        // TODO: iterate x64
        // read literal
        for (unsigned it = 0; it < literal_len; ++it) {
            bool bit = data_bits.get_uint(1);
            output_bits.store_uint(bit, 1);
            output_bits.offs += 1;
            data_bits.offs += 1;
        }

        if (data_bits.offs >= n_bits) {
            CHECK(data_bits.offs == n_bits);
            break;
        }

        const auto match_len = get_number_as_blocks(MATCH_LENGTH_BITS_BLOCK);
        const unsigned i = output_bits.offs;
        const auto bit_len = len_in_bits(i);
        const auto offset = data_bits.get_uint(bit_len);
        data_bits.offs += bit_len;
        const unsigned j = i - offset;

        // copy suffix beginning
        {
            unsigned from = j;
            unsigned to = i;
            // TODO: x64
            for (unsigned it = 0; it < match_len; ++it) {
                output_bits.offs = from;
                const bool cur = output_bits.get_uint(1);

                output_bits.offs = to;
                output_bits.store_uint(cur, 1);

                ++from;
                ++to;
            }
            output_bits.offs = to;
        }
    }

    CHECK(output_bits.offs % 8 == 0);
    std::basic_string<uint8_t> output{
        output_bits.ptr, static_cast<size_t>(output_bits.offs / 8)
    };
    return output;
}
}
