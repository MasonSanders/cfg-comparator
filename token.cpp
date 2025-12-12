#include "token.h"

Token::Token(const TokenType tt, const std::string lex)
: tokenType(tt),
  lexeme(lex)
{

}
