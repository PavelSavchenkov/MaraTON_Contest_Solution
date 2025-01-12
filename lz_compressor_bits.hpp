#pragma once
#include "utils.h"
#include "suff_aut.hpp"

namespace lz_compressor_bits {
static constexpr unsigned MIN_MATCH_LENGTH = 50; //32; 4 * 8;
static constexpr unsigned MAX_BITS_FOR_MATCH_OFFSET = 29;

std::basic_string<uint8_t> compress(
    const std::basic_string<uint8_t>& input_bytes,
    const std::basic_string<uint8_t>& dict = {}
) {
    const auto input_bits = bytes_str_to_bit_str(dict + input_bytes);

    const unsigned n = input_bits.size();
    std::vector<unsigned> best_match_suff(n, -1);
    std::vector<unsigned> best_match_len(n, 0);
    // suff aut
    {
        SuffAut<2> suff_aut_bits(input_bits);
        const auto matches = suff_aut_bits.build_matches(
            MIN_MATCH_LENGTH
        );
        for (unsigned i = 0; i < n; ++i) {
            for (const auto& match : matches[i]) {
                if (match.len > best_match_len[i]) {
                    best_match_len[i] = match.len;
                    best_match_suff[i] = match.start;
                }
            }
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

    const auto find_best_block_size = [&](const std::vector<unsigned>& nums) -> uint8_t {
        unsigned best_overall_bits = -1u;
        uint8_t best_block_size = -1;
        for (unsigned block_size = 2; block_size < (1u << 4); ++block_size) {
            unsigned overall_bits = 0;
            for (const unsigned num: nums) {
                overall_bits += (num / ((1u << block_size) - 1) + 1) * block_size;
            }
            if (overall_bits < best_overall_bits) {
                best_overall_bits = overall_bits;
                best_block_size = block_size;
            }
        }
        return best_block_size;
    };

    const unsigned start_index = dict.size() * 8;
    unsigned last_literal_start = start_index;
    std::vector<unsigned> match_lengths;
    std::vector<unsigned> literal_lengths;
    for (unsigned i = start_index; i < n;) {
        auto match_len = best_match_len[i];
        if (match_len >= MIN_MATCH_LENGTH) {
            match_lengths.push_back(match_len);
            literal_lengths.push_back(i - last_literal_start);
            i += match_len;
            last_literal_start = i;
        } else {
            ++i;
        }
    }
    if (last_literal_start < n) {
        literal_lengths.push_back(n - last_literal_start);
    }

    const uint8_t best_match_len_bits = find_best_block_size(match_lengths);
    const uint8_t best_literal_len_bits = find_best_block_size(literal_lengths);
    output_bits.store_uint(best_match_len_bits, 4);
    output_bits.offs += 4;
    output_bits.store_uint(best_literal_len_bits, 4);
    output_bits.offs += 4;

    // Push serialization bits
    last_literal_start = start_index;
    unsigned sum_length_matches = 0;
    unsigned bits_spent_on_literal_tokens = 0;
    unsigned bits_spent_on_match_len_tokens = 0;
    unsigned bits_spent_on_match_offset_tokens = 0;
    unsigned bits_spent_on_match_offset_tokens_if_optimal = 0;
    std::set<unsigned> suff_referenced;
    unsigned sum_literal_lengths = 0;
    unsigned cnt_references = 0;
    unsigned sum_offsets = 0;
    uint64_t sum_offsets_sq = 0;
    match_lengths.clear();
    for (unsigned i = start_index; i < n;) {
        auto match_len = best_match_len[i];
        if (match_len >= MIN_MATCH_LENGTH) {
            // naive check of the match
            // {
            //     unsigned from = best_match_suff[i];
            //     unsigned to = i;
            //     for (unsigned it = 0; it < match_len; ++it) {
            //         CHECK(input_bits[from + it] == input_bits[to + it]);
            //     }
            //     CHECK(input_bits[from + match_len] != input_bits[to + match_len]);
            // }
            sum_length_matches += match_len;
            suff_referenced.insert(best_match_suff[i]);
            cnt_references += 1;

            CHECK(last_literal_start <= i);
            const auto literal_len = i - last_literal_start;
            sum_literal_lengths += literal_len;
            bits_spent_on_literal_tokens += push_number_as_blocks(literal_len, best_literal_len_bits);
            for (unsigned it = 0; it < literal_len; ++it) {
                output_bits.store_uint(input_bits[last_literal_start + it], 1);
                output_bits.offs += 1;
            }
            bits_spent_on_match_len_tokens += push_number_as_blocks(match_len, best_match_len_bits);
            unsigned offset = i - best_match_suff[i];
            CHECK(offset > 0);
            --offset;
            sum_offsets += offset;
            sum_offsets_sq += offset * 1ull * offset;
            match_lengths.push_back(match_len);
            const unsigned bit_len = std::min(unsigned(len_in_bits(i - 1)), MAX_BITS_FOR_MATCH_OFFSET);
            CHECK(offset < (1u << bit_len));
            bits_spent_on_match_offset_tokens += bit_len;
            bits_spent_on_match_offset_tokens_if_optimal += len_in_bits(offset);
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
        bits_spent_on_literal_tokens += push_number_as_blocks(literal_len, best_literal_len_bits);
        for (unsigned it = 0; it < literal_len; ++it) {
            output_bits.store_uint(input_bits[last_literal_start + it], 1);
            output_bits.offs += 1;
        }
    }
    output_bits.store_uint(1, 1);
    output_bits.offs += 1;

    const std::basic_string<uint8_t> output(output_bits.ptr, static_cast<size_t>(output_bits.offs + 7) / 8);

    if (false) {
        const unsigned bits_spent_on_tokens = bits_spent_on_literal_tokens + bits_spent_on_match_len_tokens +
                                              bits_spent_on_match_offset_tokens;
        std::sort(match_lengths.begin(), match_lengths.end());
        const unsigned match_len_p90 = match_lengths[match_lengths.size() * 9 / 10];
        const double avg_offset = sum_offsets * 1.0 / cnt_references;
        const double std_offset = std::sqrt(sum_offsets_sq * 1.0 / cnt_references - avg_offset * avg_offset);
        std::cout << "SERIALIZE: " << "bits_spent_on_tokens=" << bits_spent_on_tokens
                << "\nsum_matches_len=" << sum_length_matches
                << "\nsum_matches_len in bytes: " << sum_length_matches / 8
                << "\ncnt diff suff referenced: " << suff_referenced.size()
                << "\ncnt_references: " << cnt_references
                << "\nbits_spent_on_literal_tokens: " << bits_spent_on_literal_tokens
                << "\nbits_spent_on_match_len_tokens: " << bits_spent_on_match_len_tokens
                << "\nbits_spent_on_match_offset_tokens: " << bits_spent_on_match_offset_tokens
                << "\nbits_spent_on_match_offset_tokens_if_optimal: " << bits_spent_on_match_offset_tokens_if_optimal
                << "\navg literal len: " << sum_literal_lengths * 1.0 / cnt_references
                << "\nmatch_len_p90: " << match_len_p90
                << "\navg bits per token: " << bits_spent_on_tokens * 1.0 / cnt_references
                << "\navg offset: " << avg_offset << " +- " << std_offset
                << "\noverall was " << input_bits.size() << " bits"
                << "\ncompressed into " << output.size() * 8 << " bits" << std::endl;
    }

    return output;
}

std::basic_string<uint8_t> decompress(
    const std::basic_string<uint8_t>& data,
    const std::basic_string<uint8_t>& dict = {}
) {
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

    const uint8_t best_match_len_bits = data_bits.get_uint(4);
    data_bits.offs += 4;
    const uint8_t best_literal_len_bits = data_bits.get_uint(4);
    data_bits.offs += 4;

    std::basic_string<uint8_t> output_bytes(2 << 20, 0);
    output_bytes = dict + output_bytes;
    td::BitPtr output_bits(output_bytes.data(), dict.size() * 8);
    while (data_bits.offs < n_bits) {
        const auto literal_len = get_number_as_blocks(best_literal_len_bits);

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

        const auto match_len = get_number_as_blocks(best_match_len_bits);
        const unsigned i = output_bits.offs;
        // const auto bit_len = len_in_bits(i);
        const unsigned bit_len = std::min(unsigned(len_in_bits(i - 1)), MAX_BITS_FOR_MATCH_OFFSET);
        const auto offset = data_bits.get_uint(bit_len) + 1;
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
    return output.substr(dict.size());
}
}
