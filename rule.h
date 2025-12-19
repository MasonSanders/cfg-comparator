#ifndef __RULE_H__
#define __RULE_H__

#include <vector>
#include <string>

class Symbol
{
public:
	bool isTerminal;
	std::string name;
};

class Rule
{
public:
	std::string lhs;
	std::vector<std::vector<Symbol>> rhs;
};

#endif
