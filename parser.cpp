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

#include "parser.h"
#include <iostream>
#include <cstdlib>

/*
 * Meta Grammar for parsing CFGs:
 *
 *
 * grammar -> ruleList END_OF_FILE
 * ruleList -> rule | rule ruleList
 * rule -> ID ARROW rhs SEMICOLON
 * rhs -> alternative | alternative OR rhs
 * alternative -> symbolList | EPSILON
 * symbolList -> symbol | symbol symbolList
 * symbol -> ID | STRING
 */


Parser::Parser(const std::string& input)
: lexer(Lexer(input))
{
}

Token Parser::expect(TokenType type) 
{
	Token t = lexer.getToken();
	if (t.tokenType != type)
		syntaxError();
	return t;
}

void Parser::syntaxError()
{
	std::cerr << "Syntax Error" << std::endl;
	exit(1);
}

Grammar Parser::parseGrammar()
{
	// grammar -> ruleList END_OF_FILE
	parseRuleList();
	expect(TokenType::END_OF_FILE);
	return grammar;
}

void Parser::parseRuleList()
{
	// ruleList -> rule | rule ruleList
	Rule r = parseRule();
	grammar.rules.push_back(r);

	Token t = lexer.peek();
	if (t.tokenType == TokenType::ID)
	{
		parseRuleList();
	}
}

Rule Parser::parseRule()
{
	// rule -> ID ARROW rhs SEMICOLON
	Rule r;

	Token lhsToken = expect(TokenType::ID);
	r.lhs = lhsToken.lexeme;
	if (grammar.nonterminals.find(lhsToken.lexeme) == grammar.nonterminals.end())
		grammar.nonterminals.insert(lhsToken.lexeme);

	expect(TokenType::ARROW);
	r.rhs = parseRhs();
	expect(TokenType::SEMICOLON);
	return r;
}

std::vector<std::vector<Symbol>> Parser::parseRhs()
{
	// rhs -> alternative | alternative OR rhs
	std::vector<std::vector<Symbol>> rhs_mat;
	std::vector<Symbol> alt = parseAlternative();
	rhs_mat.push_back(alt);	


	Token t = lexer.peek();
	if (t.tokenType == TokenType::OR)
	{
		expect(TokenType::OR);
		std::vector<std::vector<Symbol>> temp = parseRhs();
		rhs_mat.insert(rhs_mat.end(), temp.begin(), temp.end());
	}

	return rhs_mat;
}

std::vector<Symbol> Parser::parseAlternative()
{
	// alternative -> symbolList | EPSILON
	std::vector<Symbol> alt;

	Token t = lexer.peek();
	if (t.tokenType == TokenType::EPSILON)
	{
		t = expect(TokenType::EPSILON);
		Symbol s;
		s.isTerminal = true;
		s.name = t.lexeme;
		if (grammar.terminals.find(t.lexeme) == grammar.terminals.end())
			grammar.terminals.insert(t.lexeme);

		alt.push_back(s);
	}
	else
	{
		alt = parseSymbolList();
	}
	
	return alt;
}

std::vector<Symbol> Parser::parseSymbolList()
{
	// symbolList -> symbol | symbol symbolList
	std::vector<Symbol> symbolList;
	Symbol s = parseSymbol();
	symbolList.push_back(s);	

	Token t = lexer.peek();
	if (t.tokenType == TokenType::ID || t.tokenType == TokenType::STRING)
	{
		std::vector<Symbol> temp = parseSymbolList();
		symbolList.insert(symbolList.end(), temp.begin(), temp.end());
	}

	return symbolList;
}

Symbol Parser::parseSymbol()
{
	// symbol -> ID | STRING
	Symbol s;
	Token t = lexer.peek();
	if (t.tokenType == TokenType::ID)
	{
		t = expect(TokenType::ID);
		s.isTerminal = false;
		s.name = t.lexeme;
		if (grammar.nonterminals.find(t.lexeme) == grammar.nonterminals.end())
			grammar.nonterminals.insert(t.lexeme);
	}
	else
	{
		t = expect(TokenType::STRING);
		s.isTerminal = true;
		s.name = t.lexeme;
		if (grammar.terminals.find(t.lexeme) == grammar.terminals.end())
			grammar.terminals.insert(t.lexeme);
	}

	return s;
}





