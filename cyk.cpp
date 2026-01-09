#include "cyk.h"

/*
 * PairHash is a small helper that tells unordered_map how to hash std::pair<std::string, std::string>
 */
size_t PairHash::operator()(const std::pair<std::string, std::string>& p) const noexcept
{
    // this combines the hash of the strings into one number.
    return std::hash<std::string>{}(p.first) * 1315423911u
        ^ std::hash<std::string>{}(p.second);
}

/*
 * compute indexes to make lookup constant-time
 * termMap: terminal -> which nonterminals produce it
 * binMap: (B,C) which A produces BC
 *      If I already know something can derive B and something can derive C, which variables can derive their concatenation
 *
 */
CykIndex buildCykIndex(const Grammar& g)
{
    CykIndex idx;

    for (const auto& r : g.rules)
    {
        for (const auto& prod : r.rhs)
        {
            // if the production is a terminal production. put the production in the termMap
            if (prod.size() == 1 && prod[0].isTerminal && prod[0].name != "epsilon")
                idx.termMap[prod[0].name].insert(r.lhs);
            // if the production is a nonterminal binary production. put the production in the binMap
            else if (prod.size() == 2 && !prod[0].isTerminal && !prod[1].isTerminal)
                idx.binMap[{prod[0].name, prod[1].name}].insert(r.lhs);
        }
    }

    return idx;
}


/*
 * function to decide whether a given string is accepted by the CFG
 */
bool cykAccepts(const Grammar& g, CykIndex& idx, const std::string& startSymbol, const std::vector<std::string>& w)
{
    const size_t n = w.size();

    // if the string has a size of zero, then it must be an epsilon production.
    // return true if the start symbol does not produce epsilon, return false, else true
    if (n == 0)
    {
        for (const auto& r : g.rules)
        {
            if (r.lhs != startSymbol)
                continue;

            for (const auto& prod : r.rhs)
            {
                if (prod.size() == 1 && prod[0].isTerminal && prod[0].name == "epsilon")
                    return true;
            }
        }

        return false;
    }

    // Create the CYK DP table T
    // T[i][len] = the set of nonterminals that can generate the substring starting at position i of length len
    std::vector<std::vector<std::unordered_set<std::string>>> T(
        n, std::vector<std::unordered_set<std::string>>(n + 1)
    );

    // base case len = 1
    for (size_t i = 0; i < n; ++i)
    {
        auto it = idx.termMap.find(w[i]);

        if (it != idx.termMap.end())
            T[i][1] = it->second;
    }

    // induction: length >= 2
    for (size_t len = 2; len <= n; ++len)
    {
        for (size_t i = 0; i + len <= n; ++i)
        {
            for (size_t k = 1; k < len; ++k)
            {
                const auto& leftSet = T[i][k];
                const auto& rightSet = T[i + k][len - k];

                if (leftSet.empty() || rightSet.empty())
                    continue;

                for (const auto& B : leftSet)
                {
                    for (const auto& C : rightSet)
                    {
                        auto it = idx.binMap.find({B, C});
                        if (it != idx.binMap.end())
                            T[i][len].insert(it->second.begin(), it->second.end());
                    }
                }
            }
        }
    }

    return T[0][n].count(startSymbol) > 0;
}

std::vector<std::string> tokenizeChars(const std::string& s)
{
    std::vector<std::string> w;
    w.reserve(s.size());
    for (char c : s)
    {
        w.push_back(std::string(1, c));
    }
    return w;
}
