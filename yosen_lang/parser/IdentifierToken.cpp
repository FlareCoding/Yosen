#include "IdentifierToken.h"

namespace yosen::parser
{
    IdentifierToken::IdentifierToken(const std::string& value)
        : value(value)
    {
        type = TokenType::Identifier;
    }

    std::string IdentifierToken::to_string()
    {
        return "[TokenType: Identifier, Value: '" + value + "']";
    }
}
