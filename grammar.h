#ifndef __GRAMMAR_H__
#define __GRAMMAR_H__

#include <unordered_set>
#include "rule.h"

class Grammar
{
public:
	std::vector<Rule> rules;
	std::unordered_set<std::string> terminals;
	std::unordered_set<std::string> nonterminals;
};

#endif
