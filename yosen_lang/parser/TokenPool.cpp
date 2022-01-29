#include "TokenPool.h"
#include <YosenEnvironment.h>

namespace yosen::parser
{
	TokenRef<Token> TokenPool::next(TokenRef<Token> token)
	{
		if (!tokens.size())
		{
			YosenEnvironment::get().throw_exception(ParserException("Empty source provided"));
			return nullptr;
		}

		if (token == nullptr)
			return tokens[0];

		auto idx = index_of(token);
		if (idx == tokens.size() - 1)
			return tokens[0];

		if (idx == -1)
			return nullptr;

		return tokens[idx + 1];
	}

	TokenRef<Token> TokenPool::previous(TokenRef<Token> token)
	{
		auto idx = index_of(token);
		if (idx == -1 || idx == 0)
			return nullptr;

		return tokens[idx - 1];
	}

	TokenRef<Token> TokenPool::consume(TokenRef<Token> token)
	{
		auto next_token = next(token);

		auto idx = index_of(token);
		if (idx == -1)
			return nullptr;

		tokens.erase(tokens.begin() + idx);
		return next_token;
	}

	size_t TokenPool::index_of(TokenRef<Token> token)
	{
		for (size_t i = 0; i < tokens.size(); i++)
		{
			if (tokens[i] == token)
				return i;
		}

		return -1;
	}
}
