#include "SymbolToken.h"

namespace yosen::parser
{
    SymbolToken::SymbolToken(Symbol symbol, const std::string& value)
        : symbol(symbol), value(value)
    {
        type = TokenType::Symbol;
    }

    std::string SymbolToken::to_string()
    {
        return "[TokenType: Symbol, Value: '" + value + "']";
    }
}
