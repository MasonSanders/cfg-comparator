#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"
#include "grammar.h"

class Parser 
{
public:
	Parser(const std::string& input);
	
	Token expect(TokenType type);
	void syntaxError();
	Grammar parseGrammar();
	void parseRuleList();
	Rule parseRule();
	std::vector<std::vector<Symbol>> parseRhs();
	std::vector<Symbol> parseAlternative();
	std::vector<Symbol> parseSymbolList();
	Symbol parseSymbol();
private:
	Lexer lexer;
	Grammar grammar;
};


#endif
