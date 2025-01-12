// LOCAL

#pragma once

#include "utils.h"

td::uint8 cell_type_to_int(const CellType& type) {
    return static_cast<td::uint8>(type);
}

std::string cell_type_int_to_string(const td::uint8& type_int) {
    static const std::string type_names[] = {"Ordinary", "PrunnedBranch", "Library", "MerkleProof", "MerkleUpdate"};
    CHECK(type_int < std::size(type_names));
    return type_names[type_int];
}

td::Ref<vm::Cell> read_from_filepath_to_root_cell(const std::string& block_path) {
    std::ifstream in(block_path);

    // mode
    std::string tmp;
    in >> tmp;

    std::string base64_data;
    in >> base64_data;
    CHECK(!base64_data.empty());
    td::BufferSlice binary_data(td::base64_decode(base64_data).move_as_ok());
    td::Ref<vm::Cell> root = vm::std_boc_deserialize(binary_data).move_as_ok();
    return root;
}

unsigned cnt_occurrences(const std::basic_string<uint8_t>& sample, const std::basic_string<uint8_t>& text) {
    const auto s = sample + text;
    const unsigned n = s.size();
    std::vector<unsigned> z(n);
    unsigned l = 0;
    unsigned r = 0;
    unsigned cnt = 0;
    for (unsigned i = 1; i < n; i++) {
        if (i < r) {
            z[i] = std::min(r - i, z[i - l]);
        }
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) {
            ++z[i];
        }
        if (i + z[i] > r) {
            l = i;
            r = i + z[i];
        }
        if (i >= sample.size() && z[i] >= sample.size()) {
            ++cnt;
        }
    }
    return cnt;
}


std::basic_string<uint8_t> compress_lz_standard(const std::basic_string<uint8_t>& data) {
    auto res = td::lz4_compress(td::BufferSlice(reinterpret_cast<const char *>(data.data()), data.size()));
    return {reinterpret_cast<const uint8_t *>(res.data()), res.size()};
}

// 1 in the highest bit which is not used
inline unsigned get_min_bit_str_not_in(const std::basic_string<uint8_t>& bits) {
    CHECK(!bits.empty());
    std::vector<char> was(bits.size() * 2, 0);
    for (uint8_t len = 1; len <= bits.size(); ++len) {
        std::fill(std::begin(was), was.begin() + (1u << len), 0);
        unsigned cur = 0;
        const unsigned cover_mask = (1u << len) - 1;
        for (unsigned i = 0; i < len; ++i) {
            cur = cur * 2 + bits[i];
        }
        was[cur] = 1;
        for (unsigned i = len; i < bits.size(); ++i) {
            cur = (cur * 2 + bits[i]) & cover_mask;
            was[cur] = 1;
        }
        for (unsigned mask = 0; mask < (1u << len); ++mask) {
            if (!was[mask]) {
                return (1u << len) ^ mask;
            }
        }
    }
    CHECK(false);
    return -1u;
}

template<typename OUT, typename IN>
OUT pack_expanded_bits_to_bytes(const IN& data) {
    const auto n = data.size();
    OUT out(n / 8 + 1, 0);
    for (unsigned i = 0; i < n; ++i) {
        CHECK(0 <= data[i] && data[i] <= 1);
        if (data[i]) {
            out[i / 8] ^= 1u << (7 - i % 8);
        }
    }
    // mark the ending
    out[n / 8] ^= 1u << (7 - n % 8);
    return out;
}

inline void analyse_for_n(const unsigned n) {
    std::cout << "different codings for 0 ... " << n - 1 << " numbers" << std::endl;

    std::cout << "naive (max bytes for each): " << len_in_bytes(n - 1) * 8 << std::endl;

    std::cout << "naive (max bits for each): " << int(len_in_bits(n - 1)) << std::endl;

    std::cout << "store offsets (bytes <= bytes needed for current i) in bytes: "; {
        unsigned sum_bits = 0;
        for (unsigned i = 0; i < n; ++i) {
            sum_bits += 8 * len_in_bytes(i);
        }
        std::cout << sum_bits * 1.0 / n << std::endl;
    }

    std::cout << "store offsets (bits <= bits needed for current i) in bits: "; {
        unsigned sum_bits = 0;
        for (unsigned i = 0; i < n; ++i) {
            sum_bits += len_in_bits(i);
        }
        std::cout << sum_bits * 1.0 / n << std::endl;
    }

    for (unsigned iter = 0; iter < 2; ++iter) {
        std::cout << "first x bits define length of the number in bits (=y), so x+y_i bits overall per number";
        if (iter == 1) {
            std::cout << "but numbers <= n/2 are more probable";
        }
        std::cout << std::endl;
        const unsigned max_bits_needed = len_in_bits(n - 1);
        unsigned best_x = -1u;
        unsigned best_sum_len = -1u;
        uint8_t best_offset = 255;
        unsigned denom = 0;
        for (unsigned x = 0; x < 16; ++x) {
            uint8_t offset = 1;
            while (offset + ((1u << x) - 1) < max_bits_needed) {
                ++offset;
            }

            denom = 0;
            unsigned cur_sum = 0;
            for (unsigned i = 0; i < n; ++i) {
                unsigned coef = 1;
                if (iter == 1) {
                    if (i < n / 5) {
                        coef = 100;
                    }
                }
                cur_sum += coef * (x + std::max(offset, len_in_bits(i)));
                denom += coef;
            }

            if (cur_sum < best_sum_len) {
                best_sum_len = cur_sum;
                best_x = x;
                best_offset = offset;
            }

            std::cout << "for x=" << x << ", offset=" << int(offset) << ": " << cur_sum * 1.0 / denom << std::endl;
        }
        std::cout << "best_x = " << best_x << ", best_offset = " << int(best_offset) << ", best avg bits = " <<
                best_sum_len * 1.0 / denom << std::endl;
    }
}

struct DSU {
    std::vector<int> par{};

    explicit DSU(unsigned n) {
        par.assign(n, -1);
    }

    unsigned get_comp(const unsigned x) {
        return (par[x] < 0) ? x : (par[x] = get_comp(par[x]));
    }

    bool unite(unsigned x, unsigned y) {
        x = get_comp(x);
        y = get_comp(y);
        if (x == y) {
            return false;
        }
        if (-par[x] < -par[y]) {
            std::swap(x, y);
        }

        par[x] += par[y];
        par[y] = x;
        return true;
    }

    std::vector<unsigned> comp_sizes() {
        std::set<unsigned> comps;
        for (unsigned i = 0; i < par.size(); ++i) {
            comps.insert(get_comp(i));
        }
        std::vector<unsigned> sizes;
        for (auto c : comps) {
            sizes.push_back(-par[c]);
        }
        return sizes;
    }

    unsigned cnt_comps() {
        std::set<unsigned> comps;
        for (unsigned i = 0; i < par.size(); ++i) {
            comps.insert(get_comp(i));
        }
        return comps.size();
    }
};
