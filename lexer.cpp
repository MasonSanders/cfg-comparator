#include "lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& input)
: pos(0),
  input(input),
  hasPeeked(false),
  peekedToken(Token(TokenType::END_OF_FILE, ""))
{
}

Token Lexer::getToken()
{
	if (hasPeeked)
	{
		hasPeeked = false;
		return peekedToken;
	}

	return nextToken();
}


Token Lexer::peek()
{
	if (!hasPeeked)
	{
		peekedToken = nextToken();
		hasPeeked = true;
	}

	return peekedToken;
}

Token Lexer::nextToken()
{
	// whitespace and newline
	while (pos < input.size() && std::isspace(static_cast<unsigned_char>(input[pos])))
	{
		++pos;
	}

	// end of input
	if (pos >= input.size())
	{
		return Token(TokenType::END_OF_FILE, "");
	}

	char c = input[pos];
	
	// arrow ->

	if (c == '-' && pos + 1 < input.size() && input[pos + 1] == '>')
	{
		pos += 2;
		return Token(TokenType::ARROW, "->");
	}

	// alternation |
	
	if (c == '|') 
	{
		++pos;
		return Token(TokenType::OR, "|");
	}

	// semicolon ;
	if (c == ';')
	{
		++pos;
		return Token(TokenType::SEMICOLON, ";");
	}

	// string literal terminal
	if (c == '"')
	{
		++pos;
		size_t start = pos;
		while (pos < input.size() && input[pos] != '"')
		{
			++pos;
		}

		if (pos >= input.size())
			throw std::runtime_error("Unterminated string literal");

		std::string lexeme = input.substr(start, pos - start);
		++pos;
		return Token(TokenType::STRING, lexeme);
	}
	
	// nonterminals and epsilon
	if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
	{
		size_t start = pos;

		while (pos < input.size() && 
				(std::isalnum(static_cast<unsigned char>(input[pos])) || input[pos] == '_' ))
		{
			++pos;
		}

		std::string lexeme = input.substr(start, pos - start);

		if (lexeme == "epsilon")
			return Token(TokenType::EPSILON, lexeme);

		return Token(TokenType::ID, lexeme);
	}

	std::string msg = "Unexpected character in input: ";
	msg.push_back(c);
	throw std::runtime_error(msg);
}
