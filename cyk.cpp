#include "cyk.h"
#include <iostream>
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
bool cykAccepts(const Grammar& g, const CykIndex& idx, const std::string& startSymbol, const std::vector<std::string>& w)
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

size_t countTerminals(const std::vector<Symbol>& sentential)
{
    size_t count = 0;
    for (const auto& sym : sentential)
    {
        if (sym.isTerminal && sym.name != "epsilon")
            count++;
    }
    return count;
}

std::vector<size_t> nonterminalPositions(const std::vector<Symbol>& sentential)
{
    std::vector<size_t> pos;
    for (size_t i = 0; i < sentential.size(); ++i)
    {
        if (!sentential[i].isTerminal)
            pos.push_back(i);
    }
    return pos;
}

size_t countNonterminalsInProd(const std::vector<Symbol>& prod)
{
    size_t count = 0;
    for (const auto& sym : prod)
    {
        if (!sym.isTerminal)
            count++;
    }
    return count;
}

size_t countTerminalsInProd(const std::vector<Symbol>& prod)
{
    size_t count = 0;
    for (const auto& sym : prod)
    {
        if (sym.isTerminal && sym.name != "epsilon")
            count++;
    }
    return count;
}

size_t chooseAlternativeIndex(
    const std::vector<std::vector<Symbol>>& alts,
    std::mt19937_64& rng,
    size_t currentLen,
    size_t stepsUsed,
    const GenSettings& cfg)
{
    auto isEpsilonProd = [](const std::vector<Symbol>& prod) -> bool
    {
        return prod.size() == 1 && prod[0].isTerminal && prod[0].name == "epsilon";
    };

    std::vector<double> w(alts.size(), 1.0);

    const bool nearLenLimit = currentLen >= cfg.targetMax;
    const bool nearStepLimit = stepsUsed >= (cfg.maxSteps * 3) / 4;

    for (size_t i = 0; i < alts.size(); ++i)
    {
        const auto& prod = alts[i];

        if (isEpsilonProd(prod))
        {
            w[i] *= (currentLen < cfg.targetMin) ? 0.1 : 0.6;
            continue;
        }

        const size_t nt = countNonterminalsInProd(prod);
        const size_t tm = countTerminalsInProd(prod);

        // if near limits, prefer productions that reduce nonterminals
        if (nearLenLimit || nearStepLimit)
            w[i] *= 1.0 / (1.0 + nt);

        if (currentLen < cfg.targetMin)
            w[i] *= (1.0 + tm);

        if (currentLen > cfg.targetMax)
            w[i] *= 1.0 / (1.0 + tm);
    }

    double sum = 0.0;
    for (double x : w)
    {
        sum += x;
    }

    if (sum <= 0.0)
    {
        std::uniform_int_distribution<size_t> uni(0, alts.size() - 1);
        return uni(rng);
    }

    std::discrete_distribution<size_t> dist(w.begin(), w.end());
    return dist(rng);
}


std::optional<std::vector<std::string>> generateString(
    const RuleMap& rm,
    const std::string& startSymbol,
    std::mt19937_64& rng,
    const GenSettings& cfg)
{
    auto isEpsilonProd = [](const std::vector<Symbol>& prod) -> bool
    {
        return prod.size() == 1 && prod[0].isTerminal && prod[0].name == "epsilon";
    };

    std::vector<Symbol> sentential;
    sentential.push_back(Symbol{ false, startSymbol });

    for (size_t step = 0; step < cfg.maxSteps; ++step)
    {
        const auto nts = nonterminalPositions(sentential);

        if (nts.empty())
        {
            std::vector<std::string> out;
            out.reserve(sentential.size());

            for (const auto& s : sentential)
            {
                if (s.isTerminal && s.name != "epsilon")
                    out.push_back(s.name);
            }

            if (out.size() <= cfg.maxLen)
                return out;
            return std::nullopt;
        }

        const size_t curLen = countTerminals(sentential);
        if (curLen > cfg.maxLen)
            return std::nullopt;

        size_t pos = nts.front();
        std::uniform_real_distribution<double> coin(0.0, 1.0);

        if (coin(rng) > cfg.pLeftmost)
        {
            std::uniform_int_distribution<size_t> pick(0, nts.size() - 1);
            pos = nts[pick(rng)];
        }

        const std::string A = sentential[pos].name;

        auto it = rm.find(A);
        if (it == rm.end() || it->second.empty())
            return std::nullopt;

        const auto& alts = it->second;
        const size_t altIdx = chooseAlternativeIndex(alts, rng, curLen, step, cfg);
        const auto& prod = alts[altIdx];

        std::vector<Symbol> next;
        next.reserve(sentential.size() + prod.size());
        next.insert(next.end(), sentential.begin(), sentential.begin() + pos);

        if (!isEpsilonProd(prod))
            next.insert(next.end(), prod.begin(), prod.end());

        next.insert(next.end(), sentential.begin() + pos + 1, sentential.end());

        sentential = std::move(next);
    }

    return std::nullopt; // step limit
}

std::string joinTokens(const std::vector<std::string>& w)
{
    std::string s;
    for (const auto& t : w)
    {
        s += t;
    }
    return s;
}

RuleMap buildRuleMap(const Grammar& g)
{
    RuleMap m;
    for (const auto& r : g.rules)
    {
        m[r.lhs] = r.rhs;
    }
    return m;
}

DiffResult findCounterExample(
    const Grammar& g1,
    const std::string& s1,
    const CykIndex& idx1,
    const Grammar& g2,
    const std::string& s2,
    const CykIndex& idx2,
    size_t trials,
    uint64_t seed,
    const GenSettings& cfg)
{
    std::mt19937_64 rng(seed);

    RuleMap rm1 = buildRuleMap(g1);
    RuleMap rm2 = buildRuleMap(g2);

    std::unordered_set<std::string> seen;

    auto testOne = [&](const Grammar& genG, const RuleMap& rmG, const std::string& startG,
                       const Grammar& otherG, const std::string& startO,
                       const CykIndex& idxG, const CykIndex& idxO) -> DiffResult
    {
        for (size_t t = 0; t < trials; ++t)
        {
            auto wOpt = generateString(rmG, startG, rng, cfg);
            if (!wOpt)
                continue;

            const auto& w = *wOpt;
            std::string key = joinTokens(w);

            if (!seen.insert(key).second)
                continue;

            bool a = cykAccepts(genG, idxG, startG, w);
            bool b = cykAccepts(otherG, idxO, startO, w);

            if (!a)
            {
                std::cerr << "[WARNING] Generator produced string not accepted by its own grammar:";
                continue;
            }
            if (a != b)
                return DiffResult{true, key, a, b};
        }
        return DiffResult{};
    };

    if (auto r = testOne(g1, rm1, s1, g2, s2, idx1, idx2); r.found)
        return r;

    if (auto r = testOne(g2, rm2, s2, g1, s1, idx2, idx1); r.found)
        return r;

    return DiffResult{};
}


