#pragma once
#include "Lexer.h"
#include "AST.h"
#include <functional>

namespace yosen::parser
{	
	class Parser
	{
	public:
		Parser() = default;

		// Parses a complete string of source code
		// into an Abstract Syntax Tree (AST).
		AST parse_source(std::string& source);

		// Parses a single block of source code
		// into an AST node.
		AST parse_single_statement(std::string& source);

	private:
		// Contains all the tokens
		std::shared_ptr<TokenPool>	token_pool;

		// Specifies the current token that is being processed
		TokenRef<Token>				current_token;

		// Casts the current token to the specified token sub-type
		template <typename T = Token>
		TokenRef<T> current_token_as()
		{
			return as<T>(current_token);
		}

		// Casts the previous token to the specified token sub-type
		template <typename T = Token>
		TokenRef<T> previous_token()
		{
			return as<T>(token_pool->previous(current_token));
		}

		// Casts the next token to the specified token sub-type
		template <typename T = Token>
		TokenRef<T> next_token()
		{
			return as<T>(token_pool->next(current_token));
		}

	private:
		// Advances the current token to the next one if the current token is a
		// SymbolToken and has the given symbol.
		// Raises an exception otherwise.
		TokenRef<Token> expect(Symbol symbol);

		// Advances the current token to the next one if the current token is a
		// KeywordToken and has the given keyword.
		// Raises an exception otherwise.
		TokenRef<Token> expect(Keyword keyword);

		// Advances the current token to the next one if the current token is a
		// OperatorToken and has the given operator.
		// Raises an exception otherwise.
		TokenRef<Token> expect(Operator op);

		// Advances the current token to the next one if the current token
		// type matches the specified type.
		TokenRef<Token> expect(TokenType type);

		// Checks if the token is a symbol token and
		// whether or not it contains the specified symbol.
		bool is_symbol(TokenRef<Token> token, Symbol symbol);

		// Checks if the token is a keyword token and
		// whether or not it contains the specified keyword.
		bool is_keyword(TokenRef<Token> token, Keyword keyword);

		// Checks if the token is an operator token and
		// whether or not it contains the specified operator.
		bool is_operator(TokenRef<Token> token, Operator op);

		// Checks is the token is one of the specified stop symbols
		bool is_stop_symbol(TokenRef<Token> token, const std::initializer_list<Symbol>& stop_symbols);

	private:
		AST construct_AST();

		// Parses any top level block of code: 
		// class, function declaration, or any other statement.
		ASTNode parse_block();

		// Parses variable declarations, if/else statements,
		// loops, or expressions.
		ASTNode parse_statement();

		// Parses potential function calls, variable assignments,
		// or expressions.
		ASTNode parse_identifier();

		// Parses member variables and member functions,
		// (i.e obj.item or str.reverse())
		ASTNode parse_object_member(const std::string& parent_object);

		// Parses an expression, whether it's a binary operation,
		// function call, variable, or a literal value.
		ASTNode parse_expression(const std::initializer_list<Symbol>& stop_symbols);

		// Parses the function call
		ASTNode parse_function_call(const std::string& fn_name);

		// Parses specifically a variable declaration
		ASTNode parse_variable_declaration();
	};
}
