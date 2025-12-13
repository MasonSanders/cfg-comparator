#include "parser.h"

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

void Parser::parseGrammar()
{
	// grammar -> ruleList END_OF_FILE
	parseRuleList();
	expect(TokenType::END_OF_FILE);
}

void Parser::parseRuleList()
{
	// ruleList -> rule | rule ruleList
	parseRule();

	Token t = lexer.peek();
	if (t.tokenType == TokenType::ID)
	{
		parseRuleList();
	}
}

void Parser::parseRule()
{
	// rule -> ID ARROW rhs SEMICOLON
	expect(TokenType::ID);
	expect(TokenType::ARROW);
	parseRhs();
	expect(TokenType::SEMICOLON);
}

void Parser::parseRhs()
{
	// rhs -> alternative | alternative OR rhs
	parseAlternative();
	
	Token t = lexer.peek();
	if (t.tokenType == TokenType::OR)
	{
		expect(TokenType::OR);
		parseRhs();
	}
}

void Parser::parseAlternative()
{
	// alternative -> symbolList | EPSILON
	Token t = lexer.peek();
	if (t.tokenType == TokenType::EPSILON)
	{
		expect(TokenType::EPSILON);
	}
	else
	{
		parseSymbolList();
	}
}

void Parser::parseSymbolList()
{
	// symbolList -> symbol | symbol symbolList
	parseSymbol();
	
	Token t = lexer.peek();
	if (t.tokenType == TokenType::ID || t.tokenType == TokenType::STRING)
	{
		parseSymbolList();
	}
}

void Parser::parseSymbol()
{
	// symbol -> ID | STRING
	Token t = lexer.peek();
	if (t.tokenType == TokenType::ID)
	{
		expect(TokenType::ID);
	}
	else
	{
		expect(TokenType::STRING);
	}
}





