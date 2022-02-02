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
		Else,
		Class,
		This,
		New,
		Import,
		As
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
