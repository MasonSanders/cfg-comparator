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
	while (pos < input.size() && std::isspace(static_cast<unsigned char>(input[pos])))
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
