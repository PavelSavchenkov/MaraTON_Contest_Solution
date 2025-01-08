#pragma once

#include <array>

namespace rANS {
static constexpr uint8_t context_len = 8;
static constexpr unsigned context_vals = 1u << context_len;
static constexpr uint8_t probs_interval_bits = 12;
static const unsigned probs_interval = 1u << probs_interval_bits;
static std::array<unsigned, context_vals> PROB_ZERO;
static const uint8_t B = 62 - probs_interval_bits;

std::array<unsigned, context_vals> get_probs(const std::basic_string<uint8_t> &data) {
    td::ConstBitPtr bits(data.data(), 0);
    std::array<unsigned, context_vals> cnt_zero{};
    std::array<unsigned, context_vals> cnt_both{};
    for (unsigned i = context_len; i < data.size() * 8; ++i) {
        bits.offs = i - context_len;
        const unsigned context = bits.get_uint(context_len);
        bits.offs = i;

        bool bit = bits.get_uint(1);
        cnt_zero[context] += bit == 0;
        cnt_both[context] += 1;
    }

    std::array<unsigned, context_vals> probs{};
    for (unsigned c = 0; c < context_vals; ++c) {
        probs[c] = cnt_zero[c] * 1ull * probs_interval / cnt_both[c];
        if (probs[c] == 0) {
            probs[c] == 1;
        }
        if (probs[c] == probs_interval) {
            probs[c] = probs_interval - 1;
        }
        CHECK(0 < probs[c] && probs[c] < probs_interval);
    }
    return probs;
}

std::basic_string<uint8_t> encode(const std::basic_string<uint8_t> &data) {
    std::basic_string<uint8_t> out_buf(2 << 20, 0);
    td::BitPtr out(out_buf.data(), 0);
    const auto store_uint = [&](unsigned x, unsigned len_x) {
        out.store_uint(x, len_x);
        out.offs += len_x;
    };

    td::ConstBitPtr bits(data.data());
    // [low, high)
    uint64_t low = 0;
    uint64_t high = 1ull << B;
    const auto renormalize = [&]() {
        CHECK(low < high);
        while ((low >> B) == (high >> B)) {
            store_uint(low >> B, 1);
            low = low << 1;
            high = (high << 1) ^ 1;
        }
    };

    unsigned context = 0;
    for (unsigned i = 0; i < data.size() * 8; ++i) {
        const bool bit = bits.get_uint(1);
        bits.offs += 1;
        const unsigned prob_zero = PROB_ZERO[context];

        const uint64_t mid = low + (high - low) * prob_zero / probs_interval;
        CHECK(low < mid);
        CHECK(mid < high);
        if (bit == 0) {
            high = mid;
        } else {
            low = mid;
        }

        context = context * 2 + bit;
        context &= (1ull << context_len) - 1;

        renormalize();
    }

    store_uint(1, 1);
    while (out.offs % 8 != 0) {
        store_uint(0, 1);
    }
    const unsigned bytes = out.offs / 8;
    return {out_buf.data(), bytes};
}
};
