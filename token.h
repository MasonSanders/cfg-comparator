#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <string>

enum class TokenType {
	ID,
	STRING,
	ARROW,
	OR,
	EPSILON,
	SEMICOLON,
	END_OF_FILE
}; 


class Token
{
public:
	Token(const TokenType tt, const std::string lex);
	TokenType tokenType;
	std::string lexeme;
};

#endif
