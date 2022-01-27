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
    
    std::string SymbolToken::symbol_to_string(Symbol symbol)
    {
        switch (symbol)
        {
        case Symbol::Unknown: return "Unknown";
        case Symbol::BracketOpen: return "[";
        case Symbol::BracketClose: return "]";
        case Symbol::BraceOpen: return "{";
        case Symbol::BraceClose: return "}";
        case Symbol::ParenthesisOpen: return "(";
        case Symbol::ParenthesisClose: return ")";
        case Symbol::Comma: return ",";
        case Symbol::Period: return ".";
        case Symbol::Colon: return ":";
        case Symbol::Semicolon: return ";";
        default: return "Unknown";
        }
    }
}
