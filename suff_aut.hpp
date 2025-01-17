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

    std::vector<unsigned> last_suff;
    std::vector<unsigned> vs;
    unsigned i_ptr{};
    unsigned min_match_len{};

    // "best_suff[i] = x" means [i ... n-1] and [x .. n-1] have a common prefix of length best_len[i], x < i
    void prepare_matches(
        const unsigned min_match_len
    ) {
        const unsigned n = s.size();

        this->min_match_len = min_match_len;
        last_suff.assign(nodes.size(), -1u);
        vs.clear();
        vs.reserve(n);
        i_ptr = 0;

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
    }

    std::vector<Match> get_match(unsigned need_i) {
        CHECK(i_ptr <= need_i);

        std::vector<Match> res;
        while (i_ptr <= need_i) {
            const unsigned i = i_ptr;
            res.clear();
            CHECK(i < vs.size());
            // suff [i .. n-1] in orig indexing
            unsigned u = vs[i];
            while (u && nodes[u].len >= min_match_len) {
                if (last_suff[u] != -1u) {
                    Match match{};
                    match.start = last_suff[u];
                    match.len = nodes[u].len;
                    res.push_back(match);
                }
                last_suff[u] = i;
                u = nodes[u].link;
            }
            ++i_ptr;
        }
        return res;
    }

private:
    const std::basic_string<uint8_t>& s;
};
