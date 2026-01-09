#ifndef __CYK_H__
#define __CYK_H__

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <utility>
#include "grammar.h"

struct PairHash
{
    size_t operator()(const std::pair<std::string, std::string>& p) const noexcept;
};

struct CykIndex
{
    std::unordered_map<std::string, std::unordered_set<std::string>> termMap;
    std::unordered_map<std::pair<std::string, std::string>, std::unordered_set<std::string>, PairHash> binMap;
};

CykIndex buildCykIndex(const Grammar& g);

bool cykAccepts(const Grammar& g, const CykIndex& idx, const std::string& startSymbol, const std::vector<std::string>& w);

std::vector<std::string> tokenizeChars(const std::string& s);

#endif
