#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>
#include "token.h"

class Lexer 
{
public:
	Lexer(const std::string& input);
	Token getToken();
	Token peek();

private:
	size_t pos;
	std::string input;

	bool hasPeeked;
	Token peekedToken;
	
	Token nextToken();
};

#endif
