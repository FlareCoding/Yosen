#pragma once
#include "SymbolToken.h"
#include "IdentifierToken.h"
#include "LiteralValueToken.h"
#include "KeywordToken.h"
#include "OperatorToken.h"

namespace yosen::parser
{
	class TokenPool
	{
	public:
		TokenPool() = default;
		~TokenPool() = default;

		void add(TokenRef<Token> token) { tokens.push_back(token); }

		TokenRef<Token> next(TokenRef<Token> token);
		TokenRef<Token> previous(TokenRef<Token> token);

		// Eats the current token and returns
		// the next token in the list.
		TokenRef<Token>	consume(TokenRef<Token> token);

		std::vector<TokenRef<Token>>& get_all_tokens() { return tokens; }

	private:
		std::vector<TokenRef<Token>> tokens;

		size_t index_of(TokenRef<Token> token);
	};
}
