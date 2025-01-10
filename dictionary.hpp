// LOCAL

#pragma once

#include <map>
#include <numeric>
#include <iostream>
#include <queue>
#include <set>
#include <algorithm>

#include "utils.h"
#include "suff_aut.hpp"

template<unsigned C>
struct Dictionary : SuffAutBase<C> {
    using SuffAutBase<C>::add_c;
    using SuffAutBase<C>::nodes;
    using SuffAutBase<C>::root;
    using SuffAutBase<C>::last;

    explicit Dictionary() : SuffAutBase<C>() {};

    void add_training_data(const std::basic_string<uint8_t>& data) {
        // if (training_strings.size() >= 5) {
        //     return;
        // }

        for (const auto c: data) {
            add_c(c);
        }
        training_strings.push_back(data);
    }

    std::basic_string<uint8_t> build_dict(
        const unsigned max_dict_size,
        const unsigned min_match_length
    ) {
        const unsigned n = nodes.size();
        CHECK(n >= training_strings.size());

        std::vector<unsigned> top_sort(n - 1);
        std::iota(top_sort.begin(), top_sort.end(), 1);
        std::sort(
            top_sort.begin(),
            top_sort.end(),
            [&](unsigned a, unsigned b) {
                return nodes[a].len > nodes[b].len;
            }
        );
        CHECK(top_sort[0] == last);
        CHECK(top_sort.back() == root);

        std::cout << "top sort ready" << std::endl;

        // cnt occurrences == number of paths to the last character in the training data
        std::vector<unsigned> cnt_occured(n, 0);
        // all suffix states
        {
            unsigned v = last;
            while (v != root) {
                cnt_occured[v] = 1;
                v = nodes[v].link;
            }
        }
        for (const auto v: top_sort) {
            for (unsigned c = 0; c < C; ++c) {
                const unsigned to = nodes[v].to[c];
                if (to) {
                    CHECK(nodes[to].len > nodes[v].len);
                    cnt_occured[v] += cnt_occured[to];
                }
            }
            CHECK(cnt_occured[v] <= n);
            CHECK(cnt_occured[v] > 0);
        }

        std::cout << "cnt_occured ready" << std::endl;

        std::vector<std::vector<unsigned> > rev_to(n);
        for (unsigned v = 1; v < n; v++) {
            for (unsigned c = 0; c < C; ++c) {
                if (nodes[v].to[c]) {
                    rev_to[nodes[v].to[c]].push_back(v);
                }
            }
        }

        std::cout << "rev_to ready" << std::endl;

        const auto restore_longest_str_for_v = [&](unsigned v) {
            std::basic_string<uint8_t> s;
            const unsigned initial_v = v;
            while (v != root) {
                unsigned from = 0;
                for (const auto from_: rev_to[v]) {
                    if (nodes[v].len == nodes[from_].len + 1) {
                        from = from_;
                    }
                }
                CHECK(from != 0);

                uint8_t ch{};
                bool found_ch = false;
                for (unsigned c = 0; c < C; ++c) {
                    if (nodes[from].to[c] == v) {
                        found_ch = true;
                        ch = c;
                        break;
                    }
                }
                CHECK(found_ch);

                s += ch;
                v = from;
            }
            std::reverse(s.begin(), s.end());
            CHECK(s.size() == nodes[initial_v].len);
            return s;
        };

        // in how many distinct strings from the training set the substring is occurred
        std::vector<unsigned> cnt_diff_strings(n, 0);
        std::vector<unsigned> last_marked(n, -1u);
        std::vector<unsigned> q(n);
        for (unsigned it = 0; it < training_strings.size(); ++it) {
            const auto& data = training_strings[it];
            unsigned ql = 0;
            unsigned qr = 0;
            unsigned v = root;
            for (const auto ch: data) {
                v = nodes[v].to[ch];
                CHECK(v);
                q[qr++] = v;
                ++cnt_diff_strings[v];
                CHECK(last_marked[v] != it);
                last_marked[v] = it;
            }
            while (ql < qr) {
                v = q[ql++];

                const auto u = nodes[v].link;
                if (u && last_marked[u] != it) {
                    q[qr++] = u;
                    ++cnt_diff_strings[u];
                    last_marked[u] = it;
                }
            }
        }

        // const auto all_data = restore_longest_str_for_v(last);
        for (unsigned v = 2; v < n; ++v) {
            // const auto v_str = restore_longest_str_for_v(v);
            // unsigned real_cnt_diff = 0;
            // for (const auto& data : training_strings) {
            //     unsigned cur = cnt_occurrences(v_str, data);
            //     real_cnt_diff += (cur > 0);
            // }
            // unsigned real_cnt_occured = cnt_occurrences(v_str, all_data);
            // std::cout << "check v=" << v << ", len=" << nodes[v].len << std::endl;
            // if (real_cnt_occured != cnt_occured[v] || real_cnt_diff != cnt_diff_strings[v]) {
            //     std::cout << "bad v=" << v << ", len=" << nodes[v].len << std::endl;
            //     std::cout << "cnt_diff_strings[v]=" << cnt_diff_strings[v] << std::endl;
            //     std::cout << "cnt_occured[v]=" << cnt_occured[v] << std::endl;
            //     std::cout << "v_str=" << to_string(v_str) << std::endl;
            //     std::cout << "real_cnt_occured=" << real_cnt_occured << std::endl;
            //     std::cout << "real_cnt_diff=" << real_cnt_diff << std::endl;
            //     exit(0);
            // }
            // CHECK(real_cnt_occured == cnt_occured[v]);
            // CHECK(real_cnt_diff == cnt_diff_strings[v]);
            CHECK(cnt_diff_strings[v] <= cnt_occured[v]);
        }

        std::vector<unsigned> candidates;
        for (unsigned v = 1; v < n; v++) {
            if (
                nodes[v].len >= min_match_length &&
                nodes[v].len <= max_dict_size
                && cnt_diff_strings[v] > training_strings.size() / 2
            ) {
                candidates.push_back(v);
            }
            // std::cout << "v=" << v << ", nodes[v].len=" << nodes[v].len << ", cnt_occured=" << cnt_occured[v] << std::endl;
        }
        std::sort(
            candidates.begin(),
            candidates.end(),
            [&](const unsigned a, const unsigned b) {
                uint64_t score_a = cnt_diff_strings[a] * 1ull * (nodes[a].len - 2);
                uint64_t score_b = cnt_diff_strings[b] * 1ull * (nodes[b].len - 2);
                if (score_a != score_b) {
                    return score_a > score_b;
                }
                score_a = cnt_occured[a] * 1ull * (nodes[a].len - 2);
                score_b = cnt_occured[b] * 1ull * (nodes[b].len - 2);
                if (score_a != score_b) {
                    return score_a > score_b;
                }
                if (cnt_diff_strings[a] != cnt_diff_strings[b]) {
                    return cnt_diff_strings[a] > cnt_diff_strings[b];
                }
                return cnt_occured[a] > cnt_occured[b];
            }
        );

        std::cout << "sorted candidates ready, cnt=" << candidates.size() << std::endl;

        std::set<unsigned> dict_v;
        std::vector<char> killed(n, false);
        std::vector<char> in_dict(n, false);
        unsigned sum_len = 0;
        unsigned cnt_processed_candidates = 0;
        for (const auto candidate: candidates) {
            if (killed[candidate] || nodes[candidate].len + sum_len > max_dict_size) {
                continue;
            }
            sum_len += nodes[candidate].len;

            unsigned ql = 0;
            unsigned qr = 0;
            q[qr++] = candidate;
            while (ql < qr) {
                const unsigned v = q[ql++];

                const auto try_kill = [&](const unsigned u) {
                    if (u && !killed[u]) {
                        if (in_dict[u]) {
                            dict_v.erase(u);
                            sum_len -= nodes[u].len;
                            in_dict[u] = false;
                        }
                        killed[u] = true;
                        q[qr++] = u;
                    }
                };

                try_kill(nodes[v].link);
                for (const auto from: rev_to[v]) {
                    try_kill(from);
                }
            }

            in_dict[candidate] = true;
            dict_v.insert(candidate);
            ++cnt_processed_candidates;
            if (cnt_processed_candidates % 500 == 0) {
                std::cout << "cnt_processed_candidates = " << cnt_processed_candidates
                        << " out of " << candidates.size();
                std::cout << ", dict_v.size() = " << dict_v.size() << std::endl;
            }
        }
        CHECK(sum_len <= max_dict_size);

        std::cout << "cur_strs_in_dict = " << dict_v.size() << std::endl;
        std::basic_string<uint8_t> out;
        std::vector<std::basic_string<uint8_t> > outs;
        for (const auto v: dict_v) {
            const auto cur = restore_longest_str_for_v(v);
            out += cur;

            // CHECK(cnt_occurrences(cur, all_data) == cnt_occured[v]);
            // for (const auto& another_out : outs) {
            //     CHECK(cnt_occurrences(cur, another_out) == 0);
            // }
            // outs.push_back(cur);
        }
        for (unsigned it = 0; it < candidates.size(); ++it) {
            const auto v = candidates[candidates.size() - 1 - it];
            if (!in_dict[v]) {
                continue;
            }
            std::cout << "DICT: " << "len=" << nodes[v].len << ", cnt_diff=" << cnt_diff_strings[v]
                    << ", cnt_occur=" << cnt_occured[v] << std::endl;
        }
        CHECK(out.size() == sum_len);
        return out;
    }

private:
    std::vector<std::basic_string<uint8_t> > training_strings;
};
