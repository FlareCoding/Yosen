#include "KeywordToken.h"

namespace yosen::parser
{
	KeywordToken::KeywordToken(Keyword kwd, const std::string& value)
		: keyword(kwd), value(value)
	{
		type = TokenType::Keyword;
	}

	std::string KeywordToken::to_string()
	{
		return "[TokenType: Keyword, Value: '" + value + "']";
	}
}
