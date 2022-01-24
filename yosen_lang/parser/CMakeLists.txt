set(CMAKE_CURRENT_SOURCE_DIR parser)
set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    PARSER_HEADERS

    ${cwd}/json/json11.hpp
    ${cwd}/AST.h
    ${cwd}/Token.h
    ${cwd}/TokenPool.h
    ${cwd}/IdentifierToken.h
    ${cwd}/OperatorToken.h
    ${cwd}/SymbolToken.h
    ${cwd}/LiteralValueToken.h
    ${cwd}/Keyword.h
    ${cwd}/Operator.h
    ${cwd}/Symbol.h
    ${cwd}/Lexer.h
    ${cwd}/Parser.h

    PARENT_SCOPE
)

set(
    PARSER_SOURCES

    ${cwd}/json/json11.cpp
    ${cwd}/Token.cpp
    ${cwd}/TokenPool.cpp
    ${cwd}/IdentifierToken.cpp
    ${cwd}/OperatorToken.cpp
    ${cwd}/KeywordToken.cpp
    ${cwd}/SymbolToken.cpp
    ${cwd}/LiteralValueToken.cpp
    ${cwd}/Lexer.cpp
    ${cwd}/Parser.cpp

    PARENT_SCOPE
)