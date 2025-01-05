#pragma once

#include <map>
#include <numeric>
#include <iostream>
#include <queue>
#include <set>
#include <algorithm>

#include "utils.h"

struct Dictionary {
    explicit Dictionary() {
        root = new_node();
        last = root;
    }

    void add_training_data(const std::basic_string<uint8_t> &data) {
        for (const auto &c: data) {
            add_c(c);
        }
        training_strings.push_back(data);
    }

    std::basic_string<uint8_t> build_dict(const unsigned max_size) {
        const unsigned n = nodes.size();
        CHECK(n > 1);

        std::vector<unsigned> top_sort(n);
        std::iota(top_sort.begin(), top_sort.end(), 0);
        std::sort(
            top_sort.begin(),
            top_sort.end(),
            [&](unsigned a, unsigned b) {
                return nodes[a].len > nodes[b].len;
            }
        );
        CHECK(top_sort[0] == last);

        std::vector<std::vector<unsigned> > rev_to(n);
        for (unsigned v = 0; v < n; v++) {
            for (const auto &it: nodes[v].to) {
                unsigned to = it.second;
                rev_to[to].push_back(v);
            }
        }

        // cnt occurrences == number of paths to the last character in the training data
        std::vector<unsigned> cnt_occured(n, 0);
        cnt_occured[last] = 1;
        for (unsigned i = 1; i < n; ++i) {
            const unsigned v = top_sort[i];
            for (const auto &it: nodes[v].to) {
                const unsigned to = it.second;
                cnt_occured[v] += cnt_occured[to];
            }
        }

        // in how many distinct strings from the training set the substring is occurred
        std::vector<unsigned> cnt_diff_strings(n, 0);
        cnt_diff_strings[root] = training_strings.size();
        for (const auto &data: training_strings) {
            std::unordered_set<unsigned> marked;
            std::queue<unsigned> q;
            unsigned v = root;
            for (const auto &ch: data) {
                CHECK(nodes[v].to.count(ch));
                v = nodes[v].to[ch];
                q.push(v);
                ++cnt_diff_strings[v];
                marked.insert(v);
            }
            while (!q.empty()) {
                v = q.front();
                q.pop();

                if (nodes[v].link != -1u && !marked.count(nodes[v].link)) {
                    q.push(nodes[v].link);
                    ++cnt_diff_strings[nodes[v].link];
                    marked.insert(nodes[v].link);
                }

                for (const auto from: rev_to[v]) {
                    if (!marked.count(from)) {
                        q.push(from);
                        ++cnt_diff_strings[from];
                        marked.insert(from);
                    }
                }
            }
        }

        std::vector<unsigned> candidates;
        for (unsigned v = 0; v < n; v++) {
            if (nodes[v].len >= 4 && nodes[v].len <= max_size && cnt_occured[v] > 1) {
                candidates.push_back(v);
            }
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

        std::set<unsigned> dict_v;
        std::vector<char> killed(n, false);
        std::vector<char> in_dict(n, false);
        unsigned sum_len = 0;
        for (const auto candidate: candidates) {
            if (killed[candidate]) {
                continue;
            }

            std::set<unsigned int> to_kill;
            // set of substrings of the candidate which are not already killed, but will be if cand is taken
            {
                std::queue<unsigned> q;
                q.push(candidate);

                auto try_push = [&](unsigned v) {
                    if (v != -1u && !to_kill.count(v) && !killed[v]) {
                        to_kill.insert(v);
                        q.push(v);
                    }
                };

                while (!q.empty()) {
                    const unsigned v = q.front();
                    q.pop();

                    try_push(nodes[v].link);
                    for (const auto from: rev_to[v]) {
                        try_push(from);
                    }
                }
            }

            unsigned new_sum_len = sum_len + nodes[candidate].len;
            for (const auto v: to_kill) {
                if (in_dict[v]) {
                    CHECK(nodes[v].len <= new_sum_len);
                    new_sum_len -= nodes[v].len;
                }
            }
            if (new_sum_len <= max_size) {
                in_dict[candidate] = true;
                dict_v.insert(candidate);
                for (const auto v: to_kill) {
                    killed[v] = true;
                    if (in_dict[v]) {
                        in_dict[v] = false;
                        dict_v.erase(v);
                    }
                }
                sum_len = new_sum_len;
            }
            // std::cout << "cur_strs_in_dict = " << dict_v.size() << std::endl;
            // std::cout << "cand len = " << nodes[candidate].len << ", cand cnt = " << cnt_occured[candidate] << std::endl;
            // std::cout << "them: ";
            // for (const auto v : dict_v) {
            //     std::cout << "(len=" << nodes[v].len << ", cnt=" << cnt_occured[v] << ") ";
            // }
            // std::cout << "\n---" << std::endl;
        }
        CHECK(sum_len <= max_size);

        const auto restore_longest_str_for_v = [&](unsigned v) {
            std::basic_string<uint8_t> s;
            const unsigned initial_v = v;
            while (v != root) {
                unsigned from = -1u;
                for (const auto from_: rev_to[v]) {
                    if (nodes[v].len == nodes[from_].len + 1) {
                        from = from_;
                    }
                }
                CHECK(from != -1u);

                uint8_t ch{};
                bool found_ch = false;
                for (const auto &it: nodes[from].to) {
                    if (it.second == v) {
                        found_ch = true;
                        ch = it.first;
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

        const auto all_data = restore_longest_str_for_v(last);

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
        for (const unsigned v: dict_v) {
            std::cout << "DICT: " << "len=" << nodes[v].len << ", cnt_diff=" << cnt_diff_strings[v] << std::endl;
        }
        CHECK(out.size() == sum_len);
        return out;
    }

private:
    struct Node {
        std::map<uint8_t, unsigned> to;
        unsigned link{-1u};
        unsigned len{};

        explicit Node() = default;
    };

    unsigned new_node() {
        nodes.emplace_back();
        return nodes.size() - 1;
    }

    unsigned new_node(Node from) {
        unsigned idx = new_node();
        nodes[idx] = from;
        return idx;
    }

    void add_c(uint8_t c) {
        // std::cout << "add c = " << int(c) << std::endl;
        unsigned cur = new_node();
        nodes[cur].len = nodes[last].len + 1;
        unsigned p = last;
        last = cur;
        for (; p != -1u && !nodes[p].to.count(c); p = nodes[p].link) {
            nodes[p].to[c] = cur;
        }

        if (p == -1u) {
            nodes[cur].link = root;
            return;
        }

        unsigned q = nodes[p].to[c];
        if (nodes[q].len == nodes[p].len + 1) {
            nodes[cur].link = q;
            return;
        }

        unsigned clone = new_node(nodes[q]);
        nodes[clone].len = nodes[p].len + 1;
        nodes[cur].link = nodes[q].link = clone;
        for (; p != -1u && nodes[p].to.count(c) && nodes[p].to[c] == q; p = nodes[p].link) {
            nodes[p].to[c] = clone;
        }
    }

    std::vector<Node> nodes;
    std::vector<std::basic_string<uint8_t> > training_strings;
    unsigned last{-1u};
    unsigned root{-1u};
};
