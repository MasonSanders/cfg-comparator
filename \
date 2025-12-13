#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

class Parser 
{
public:
	Parser(const std::string& input);
	
	Token expect(TokenType type);
	void syntaxError();
	void parseGrammar();
	void parseRuleList();
	void parseRule();
	void parseRhs();
	void parseAlternative();
	void parseSymbolList();
	void parseSymbol();
private:
	Lexer lexer;
};


#endif
