#pragma once
#include "Token.h"

namespace yosen::parser
{
	enum class Symbol
	{
		Unknown = -1,

		BracketOpen, BracketClose,
		BraceOpen, BraceClose,
		ParenthesisOpen, ParenthesisClose,

		Comma, Period, Colon, Semicolon
	};

	class SymbolToken : public Token
	{
	public:
		SymbolToken(Symbol symbol = Symbol::Unknown, const std::string& value = "");
		std::string to_string() override;

		Symbol		symbol;
		std::string value;

		static std::string symbol_to_string(Symbol symbol);
	};
}
