cmake_minimum_required(VERSION 3.0)
set(CMAKE_CURRENT_SOURCE_DIR src/parser)

set(cwd ${CMAKE_CURRENT_SOURCE_DIR})

set(
    PARSER_HEADERS

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