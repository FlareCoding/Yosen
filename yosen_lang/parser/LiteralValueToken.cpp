#include "LiteralValueToken.h"

namespace yosen::parser
{
    LiteralValueToken::LiteralValueToken(LiteralType value_type, const std::string& value)
        : value_type(value_type), value(value)
    {
        type = TokenType::LiteralValue;
    }

    std::string LiteralValueToken::to_string()
    {
        return "[TokenType: LiteralValue, Value: '" + value + "' ValueType: '" + value_string_map[value_type] + "']";
    }

    std::string LiteralValueToken::type_to_string(LiteralType type)
    {
        switch (type)
        {
        case LiteralType::Null: return "null";
        case LiteralType::Boolean: return "bool";
        case LiteralType::Integer: return "int";
        case LiteralType::String: return "string";
        case LiteralType::Char: return "char";
        case LiteralType::Float: return "float";
        case LiteralType::List: return "list";
        default: return "unknown";
        }
    }
    
    LiteralType LiteralValueToken::type_from_string(const std::string& str)
    {
        if (str == "null") return LiteralType::Null;
        if (str == "bool") return LiteralType::Boolean;
        if (str == "int") return LiteralType::Integer;
        if (str == "string") return LiteralType::String;
        if (str == "char") return LiteralType::Char;
        if (str == "float") return LiteralType::Float;
        if (str == "list") return LiteralType::List;

        return LiteralType::Null;
    }
}
