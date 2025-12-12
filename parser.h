#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

class Parser 
{
public:
	Parser(const std::string& input);
	
	Token expect(Token t);
	void syntaxError();
	void parseGrammar();

private:
	Lexer lexer;
};


#endif
