#pragma once


template<unsigned C>
struct SuffAutBase {
    explicit SuffAutBase() {
        CHECK(C <= 256);
        new_node(); // so id=0 is NULL
        root = new_node();
        last = root;
    }

protected:
    struct Node {
        std::array<unsigned, C> to{};
        unsigned link{};
        unsigned len{};
    };

    std::vector<Node> nodes;
    unsigned root{};
    unsigned last{};

    unsigned new_node() {
        nodes.emplace_back();
        return nodes.size() - 1;
    }

    unsigned new_node(const Node from) {
        const unsigned idx = new_node();
        nodes[idx] = from;
        return idx;
    }

    void add_c(const uint8_t c) {
        CHECK(c < C);
        unsigned cur = new_node();
        nodes[cur].len = nodes[last].len + 1;
        unsigned p = last;
        last = cur;
        for (; p && !nodes[p].to[c]; p = nodes[p].link) {
            nodes[p].to[c] = cur;
        }

        if (!p) {
            nodes[cur].link = root;
            return;
        }

        const unsigned q = nodes[p].to[c];
        if (nodes[q].len == nodes[p].len + 1) {
            nodes[cur].link = q;
            return;
        }

        const unsigned clone = new_node(nodes[q]);
        nodes[clone].len = nodes[p].len + 1;
        nodes[cur].link = nodes[q].link = clone;
        for (; p && nodes[p].to[c] == q; p = nodes[p].link) {
            nodes[p].to[c] = clone;
        }
    }
};

template<unsigned C>
struct SuffAut : SuffAutBase<C> {
    using SuffAutBase<C>::add_c;
    using SuffAutBase<C>::nodes;
    using SuffAutBase<C>::root;

    explicit SuffAut(const std::basic_string<uint8_t> &s) : SuffAutBase<C>(), s(s) {
        nodes.reserve(s.size() * 2);

        for (unsigned it = 0; it < s.size(); ++it) {
            add_c(s[s.size() - 1 - it]);
        }
    }

    // "best_suff[i] = x" means [i ... n-1] and [x .. n-1] have a common prefix of length best_len[i], x < i
    void build_matches(
        std::vector<unsigned> &best_suff,
        std::vector<unsigned> &best_len,
        const unsigned min_match_len,
        const unsigned max_suff_offset // only (i - x) <= max_suff_offset
    ) {
        const unsigned n = s.size();
        CHECK(n <= best_suff.size());
        CHECK(n <= best_len.size());

        std::vector<unsigned> vs;
        vs.reserve(n);
        // vs: nodes corresponding to prefixes of the reversed string
        {
            unsigned v = root;
            for (unsigned it = 0; it < n; ++it) {
                CHECK(v);
                v = nodes[v].to[s[n - 1 - it]];
                vs.push_back(v);
            }
            CHECK(vs.size() == n);
            std::reverse(vs.begin(), vs.end());
        }

        std::vector<unsigned> last_suff(nodes.size(), -1); // in orig indexing, before string reversal
        // from the longest orig suff to shortest
        for (unsigned i = 0; i < n; ++i) {
            best_suff[i] = -1u;
            best_len[i] = 0;
            // suff [i .. n-1] in orig indexing
            unsigned u = vs[i];
            while (u && nodes[u].len >= min_match_len) {
                if (last_suff[u] != -1u) {
                    CHECK(i - last_suff[u] <= max_suff_offset);
                    if (i - last_suff[u] <= max_suff_offset && nodes[u].len > best_len[i]) {
                        CHECK(best_suff[i] == -1u);
                        best_suff[i] = last_suff[u];
                        best_len[i] = nodes[u].len;
                    }
                }
                last_suff[u] = i;
                u = nodes[u].link;
            }
        }
        for (unsigned i = 0; i < n; ++i) {
            if (best_len[i] >= min_match_len) {
                CHECK(best_suff[i] != -1u);
                CHECK(i - best_suff[i] <= max_suff_offset);
            }
        }
    }

private:
    const std::basic_string<uint8_t> &s;
};
