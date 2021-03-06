#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace yosen::parser
{
	enum class TokenType
	{
		Empty,
		Identifier,
		LiteralValue,
		Operator,
		Keyword,
		Symbol,
		EOFToken
	};

	class Token
	{
	public:
		size_t		lineno = 1;
		TokenType	type = TokenType::Empty;

		virtual std::string to_string();

		static std::string token_type_to_string(TokenType type);
	};

	class EOFToken : public Token
	{
	public:
		EOFToken() { type = TokenType::EOFToken; }
	};

	template <typename T>
	using TokenRef = std::shared_ptr<T>;

	template <typename T, typename... Args>
	constexpr TokenRef<T> make_token(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	constexpr TokenRef<T> as(Args&&... args)
	{
		return std::static_pointer_cast<T>(std::forward<Args>(args)...);
	}
}
