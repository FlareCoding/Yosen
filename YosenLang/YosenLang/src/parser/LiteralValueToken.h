#pragma once
#include "Token.h"

namespace yosen::parser
{
	enum class LiteralType
	{
		Null,
		Integer,
		Float,
		String,
		Char,
		Boolean,
		Pointer
	};

	class LiteralValueToken : public Token
	{
	public:
		LiteralValueToken(LiteralType value_type = LiteralType::Null, const std::string& value = "");
		std::string to_string() override;

		LiteralType value_type;
		std::string value;

		static std::string type_to_string(LiteralType type);
		static LiteralType type_from_string(const std::string& str);

	private:
		std::unordered_map<LiteralType, std::string> value_string_map = {
			{ LiteralType::Null,	"Null" },
			{ LiteralType::Float,	"Float" },
			{ LiteralType::Integer, "Integer" },
			{ LiteralType::String,	"String" },
			{ LiteralType::Char,	"Char" },
			{ LiteralType::Pointer,	"Pointer" },
		};
	};
}
