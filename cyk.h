/*
 *    Copyright (C) 2025  Mason Sanders
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __CYK_H__
#define __CYK_H__

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <random>
#include <optional>
#include <algorithm>
#include <utility>
#include "grammar.h"

using RuleMap = std::unordered_map<std::string, std::vector<std::vector<Symbol>>>;

struct PairHash
{
    size_t operator()(const std::pair<std::string, std::string>& p) const noexcept;
};

struct CykIndex
{
    std::unordered_map<std::string, std::unordered_set<std::string>> termMap;
    std::unordered_map<std::pair<std::string, std::string>, std::unordered_set<std::string>, PairHash> binMap;
};

// settings for generating strings
struct GenSettings
{
    size_t maxSteps = 200; // prevents infinite derivations
    size_t maxLen = 50; // max terminals in output
    size_t targetMin = 1; // encourage lengths in this range
    size_t targetMax = 20;
    double pLeftmost = 0.8; // 80% expand leftmost NT, else random NT
};

struct DiffResult
{
    bool found = false;
    std::string witness;
    bool g1Accepts = false;
    bool g2Accepts = false;
};

CykIndex buildCykIndex(const Grammar& g);

bool cykAccepts(
    const Grammar& g,
    const CykIndex& idx,
    const std::string& startSymbol,
    const std::vector<std::string>& w);

std::vector<std::string> tokenizeChars(const std::string& s);

RuleMap buildRuleMap(const Grammar& g);

size_t countTerminals(const std::vector<Symbol>& sentential);

std::vector<size_t> nonterminalPositions(const std::vector<Symbol>& sentential);

size_t countNonterminalsInProd(const std::vector<Symbol>& prod);

size_t countTerminalsInProd(const std::vector<Symbol>& prod);

size_t chooseAlternativeIndex(
    const std::vector<std::vector<Symbol>>& alts,
    std::mt19937_64& rng,
    size_t currentLen,
    size_t stepsUsed,
    const GenSettings& cfg);

std::optional<std::vector<std::string>> generateString(
    const RuleMap& rm,
    const std::string& startSymbol,
    std::mt19937_64& rng,
    const GenSettings& cfg);

std::string joinTokens(const std::vector<std::string>& w);

DiffResult findCounterExample(
    const Grammar& g1,
    const std::string& s1,
    const CykIndex& idx1,
    const Grammar& g2,
    const std::string& s2,
    const CykIndex& idx2,
    size_t trials,
    uint64_t seed,
    const GenSettings& cfg);




#endif
