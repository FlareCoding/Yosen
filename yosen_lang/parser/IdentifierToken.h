#pragma once
#include "Token.h"

namespace yosen::parser
{
	class IdentifierToken : public Token
	{
	public:
		IdentifierToken(const std::string& value = "");
		std::string to_string() override;

		std::string value;
	};
}
