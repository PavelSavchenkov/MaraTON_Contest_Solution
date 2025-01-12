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

struct Match {
    unsigned start{-1u};
    unsigned len{-1u};
};

template<unsigned C>
struct SuffAut : SuffAutBase<C> {
    using SuffAutBase<C>::add_c;
    using SuffAutBase<C>::nodes;
    using SuffAutBase<C>::root;

    explicit SuffAut(const std::basic_string<uint8_t>& s) : SuffAutBase<C>(), s(s) {
        nodes.reserve(s.size() * 2);

        for (unsigned it = 0; it < s.size(); ++it) {
            add_c(s[s.size() - 1 - it]);
        }
    }

    // "best_suff[i] = x" means [i ... n-1] and [x .. n-1] have a common prefix of length best_len[i], x < i
    std::vector<std::vector<Match>>
    build_matches(
        const unsigned min_match_len
    ) {
        const unsigned n = s.size();
        std::vector<std::vector<Match>> matches(n);

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
            // suff [i .. n-1] in orig indexing
            unsigned u = vs[i];
            while (u && nodes[u].len >= min_match_len) {
                if (last_suff[u] != -1u) {
                    Match match{};
                    match.start = last_suff[u];
                    match.len = nodes[u].len;
                    matches[i].push_back(match);
                }
                last_suff[u] = i;
                u = nodes[u].link;
            }
        }
        return matches;
    }

private:
    const std::basic_string<uint8_t>& s;
};
