// LOCAL
#pragma once

#include <array>
#include <numeric>

namespace rANS {
static unsigned TOTAL_FREQ = 1u << 12;
static auto BOUND_X = static_cast<uint64_t>(1) << (64 - 16);

std::vector<unsigned> normalize_freq(std::vector<unsigned> freq) {
    for (auto& f: freq) {
        if (f == 0) {
            f = 1;
        }
    }

    unsigned total_freq_initial = std::accumulate(freq.begin(), freq.end(), 0);
    for (auto& f: freq) {
        f = f * 1ull * TOTAL_FREQ / total_freq_initial;
    }

    unsigned total_freq_new = std::accumulate(freq.begin(), freq.end(), 0);
    CHECK(total_freq_new <= TOTAL_FREQ);
    while (total_freq_new < TOTAL_FREQ) {
        for (auto& f: freq) {
            if (total_freq_new < TOTAL_FREQ) {
                f += 1;
                total_freq_new += 1;
            }
        }
    }
    return freq;
}

std::vector<unsigned> get_cum_freq(const std::vector<unsigned>& freq) {
    std::vector<unsigned> cum_freq(freq.size() + 1, 0);
    for (unsigned i = 1; i <= freq.size(); i++) {
        cum_freq[i] = cum_freq[i - 1] + freq[i - 1];
    }
    return cum_freq;
}

struct Arith {
    explicit Arith(const std::vector<unsigned>& freq, const unsigned seq_len) :
        freq(normalize_freq(freq)),
        cum_freq(
            get_cum_freq(this->freq)
        ),
        seq_len(seq_len)
    {
    }

protected:
    const std::vector<unsigned> freq;
    const std::vector<unsigned> cum_freq;
    const unsigned seq_len;
};

struct ArithEncoder : Arith {
    explicit ArithEncoder(const std::vector<unsigned>& freq, const unsigned seq_len) : Arith(freq, seq_len)
    {
    }

    void shift_x(td::BitPtr& ptr) {
        ptr.store_uint(x & 255, 8);
        ptr.offs += 8;
        x >>= 8;
    }

    // ptr should be the same for all calls
    void encode_symbol(td::BitPtr& ptr, const unsigned s) {
        CHECK(s < freq.size());
        CHECK(symbols_added < seq_len);
        CHECK(x >= 1);

        // x-th number in s-sequence
        x = ((x - 1) / freq[s]) * TOTAL_FREQ + cum_freq[s] + (x - 1) % freq[s] + 1;
        ++symbols_added;

        while (x >= BOUND_X) {
            shift_x(ptr);
        }

        if (symbols_added == seq_len) {
            for (unsigned it = 0; it < 64 / 8; ++it) {
                shift_x(ptr);
            }
        }
    }

private:
    unsigned symbols_added{};

    uint64_t x = 1;
};

struct ArithDecoder : Arith {
    explicit ArithDecoder(const std::vector<unsigned>& freq, const unsigned seq_len) : Arith(freq, seq_len)
    {
    }

    void shift_x(td::BitPtr& ptr) {
        ptr.store_uint(x & 255, 8);
        ptr.offs += 8;
        x >>= 8;
    }

    void update_x(td::BitPtr& ptr) {
        uint64_t byte = ptr.get_uint(8);
        ptr.offs += 8;
    }

    // ptr should be the same for all calls
    void decode_symbol(td::BitPtr& ptr) {
        // x-th number in s-sequence
        // x = ((x - 1) / freq[s]) * TOTAL_FREQ + cum_freq[s] + (x - 1) % freq[s] + 1;

        // which s we correspond to
        CHECK(x >= 1);
        uint64_t y = (x - 1) % TOTAL_FREQ;
        // max s | cum_freq[s] <= y
        unsigned L = 0;
        unsigned R = freq.size();
        while (L + 1 != R) {
            unsigned M = (L + R) / 2;
            if (cum_freq[M] <= y) {
                L = M;
            } else {
                R = M;
            }
        }
        const unsigned s = L;
        CHECK(cum_freq[s] <= y);

        // what number we are in s-sequence
        x = ((x - 1) / TOTAL_FREQ) * freq[s] + (x - 1) % cum_freq[s];
    }

private:
    unsigned symbols_decoded{};
    uint64_t x{};
};

};
