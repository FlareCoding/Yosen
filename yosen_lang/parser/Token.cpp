#include "Token.h"

namespace yosen::parser
{
	std::string Token::to_string()
	{
		return "[TokenType: Empty, Value: '']";
	}
	
	std::string Token::token_type_to_string(TokenType type)
	{
		switch (type)
		{
		case TokenType::Empty: return "Empty";
		case TokenType::Identifier: return "Identifier";
		case TokenType::LiteralValue: return "LiteralValue";
		case TokenType::Operator: return "Operator";
		case TokenType::Keyword: return "Keyword";
		case TokenType::Symbol: return "Symbol";
		default: return "Unknown";
		}
	}
}
