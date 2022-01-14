#pragma once
#include "Token.h"
#include "Operator.h"

namespace yosen::parser
{
	class OperatorToken : public Token
	{
	public:
		OperatorToken(Operator op = Operator::Unknown, const std::string& value = "");
		std::string to_string() override;

		Operator	op;
		std::string value;
	};
}
