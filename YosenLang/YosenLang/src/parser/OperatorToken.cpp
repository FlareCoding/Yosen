#include "OperatorToken.h"

namespace yosen::parser
{
	OperatorToken::OperatorToken(Operator op, const std::string& value)
		: op(op), value(value)
	{
		type = TokenType::Operator;
	}

	std::string OperatorToken::to_string()
	{
		return "[TokenType: Operator, Value: '" + value + "']";
	}
}
