#pragma once
#include "Token.h"

namespace yosen::parser
{
	enum class Keyword
	{
		Unknown = -1,
		Var,
		Func,
		Return,
		While,
		For,
		Break,
		If,
		Elif,
		Else,
		Class,
		This,
		New
	};

	class KeywordToken : public Token
	{
	public:
		KeywordToken(Keyword kwd = Keyword::Unknown, const std::string& value = "");
		std::string to_string() override;

		Keyword		keyword;
		std::string value;
	};
}
