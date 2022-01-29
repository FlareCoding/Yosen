#include "Parser.h"
#include <YosenEnvironment.h>

namespace yosen::parser
{
	static bool IsBinaryOperator(Operator op)
	{
		switch (op)
		{
		case Operator::Add:
		case Operator::Sub:
		case Operator::Mul:
		case Operator::Div:
			return true;
		default:
			return false;
		}
	}

	static bool IsBooleanOperator(Operator op)
	{
		switch (op)
		{
		case Operator::LessThan:
		case Operator::LessThanOrEqual:
		case Operator::GreaterThan:
		case Operator::GreaterThanOrEqual:
		case Operator::Equequ:
		case Operator::Notequ:
		case Operator::Not:
		case Operator::And:
		case Operator::Or:
			return true;
		default:
			return false;
		}
	}

	AST Parser::parse_source(std::string& source)
	{
		Lexer lexer;
		token_pool = lexer.construct_token_pool(source);
		current_token = token_pool->next(nullptr);

		return construct_AST();
	}

	AST Parser::parse_single_statement(std::string& source)
	{
		Lexer lexer;
		token_pool = lexer.construct_token_pool(source);
		current_token = token_pool->next(nullptr);

		return AST(parse_statement());

	}

	TokenRef<Token> Parser::expect(Symbol symbol)
	{
		if (current_token->type != TokenType::Symbol)
		{
			auto ex_reason = "Line " + std::to_string(current_token->lineno) + 
							 " - expected a symbol \"" + 
							 SymbolToken::symbol_to_string(symbol) + "\"";

			YosenEnvironment::get().throw_exception(ParserException(ex_reason));
			return current_token;
		}

		if (as<SymbolToken>(current_token)->symbol != symbol)
		{
			auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - unexpected symbol found";
			YosenEnvironment::get().throw_exception(ParserException(ex_reason));
			return current_token;
		}

		auto result = current_token;
		current_token = token_pool->next(current_token);

		return result;
	}
	
	TokenRef<Token> Parser::expect(Keyword keyword)
	{
		if (current_token->type != TokenType::Keyword)
		{
			auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - expected a keyword";
			YosenEnvironment::get().throw_exception(ParserException(ex_reason));
			return current_token;
		}

		if (as<KeywordToken>(current_token)->keyword != keyword)
		{
			auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - unexpected keyword found";
			YosenEnvironment::get().throw_exception(ParserException(ex_reason));
			return current_token;
		}

		auto result = current_token;
		current_token = token_pool->next(current_token);

		return result;
	}
	
	TokenRef<Token> Parser::expect(Operator op)
	{
		if (current_token->type != TokenType::Operator)
		{
			auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - expected an operator";
			YosenEnvironment::get().throw_exception(ParserException(ex_reason));
			return current_token;
		}

		if (as<OperatorToken>(current_token)->op != op)
		{
			auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - unexpected operator found";
			YosenEnvironment::get().throw_exception(ParserException(ex_reason));
			return current_token;
		}

		auto result = current_token;
		current_token = token_pool->next(current_token);

		return result;
	}
	
	TokenRef<Token> Parser::expect(TokenType type)
	{
		if (current_token->type != type)
		{
			auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - expected symbol: " + Token::token_type_to_string(type);
			YosenEnvironment::get().throw_exception(ParserException(ex_reason));
			return current_token;
		}

		auto result = current_token;
		current_token = token_pool->next(current_token);

		return result;
	}
	
	bool Parser::is_symbol(TokenRef<Token> token, Symbol symbol)
	{
		if (token->type != TokenType::Symbol)
			return false;

		if (as<SymbolToken>(token)->symbol != symbol)
			return false;

		return true;
	}
	
	bool Parser::is_keyword(TokenRef<Token> token, Keyword keyword)
	{
		if (token->type != TokenType::Keyword)
			return false;

		if (as<KeywordToken>(token)->keyword != keyword)
			return false;

		return true;
	}
	
	bool Parser::is_operator(TokenRef<Token> token, Operator op)
	{
		if (token->type != TokenType::Operator)
			return false;

		if (as<OperatorToken>(token)->op != op)
			return false;

		return true;
	}

	bool Parser::is_stop_symbol(TokenRef<Token> token, const std::initializer_list<Symbol>& stop_symbols)
	{
		for (auto& symbol : stop_symbols)
			if (is_symbol(token, symbol))
				return true;

		return false;
	}
	
	AST Parser::construct_AST()
	{
		json11::Json::array nodes;
		
		while (current_token && current_token->type != TokenType::EOFToken)
		{
			auto node = parse_block();
			nodes.push_back(node);
		}

		return json11::Json(nodes);
	}
	
	ASTNode Parser::parse_block()
	{
		if (is_keyword(current_token, Keyword::Import))
			return parse_import_statement();

		if (is_keyword(current_token, Keyword::Func))
			return parse_function_declaration();

		return parse_statement();
	}

	ASTNode Parser::parse_statement()
	{
		ASTNode node;

		if (is_keyword(current_token, Keyword::Import))
			node = parse_import_statement();

		else if (is_keyword(current_token, Keyword::Var))
			node = parse_variable_declaration();

		else if (current_token->type == TokenType::Identifier)
			node = parse_identifier();

		// Check for semicolon presence
		if (is_symbol(current_token, Symbol::Semicolon))
			expect(Symbol::Semicolon);

		return node;
	}

	ASTNode Parser::parse_import_statement()
	{
		expect(Keyword::Import);

		// Get the imported library name
		auto lib_name = as<IdentifierToken>(current_token)->value;
		expect(TokenType::Identifier);
		expect(Symbol::Semicolon);

		ASTNode node;
		node["type"] = ASTNodeType_Import;
		node["library"] = lib_name;

		return node;
	}
	
	ASTNode Parser::parse_identifier()
	{
		// Get the identifier value
		auto identifier = as<IdentifierToken>(current_token)->value;
		expect(TokenType::Identifier);

		// Check for namespace presence
		while (is_operator(current_token, Operator::Namespace))
		{
			expect(Operator::Namespace);
			identifier.append("::");

			auto next_id_token = expect(TokenType::Identifier);
			identifier.append(as<IdentifierToken>(next_id_token)->value);
		}

		// Check for child elements or member functions (i.e obj.item or str.reverse())
		if (is_symbol(current_token, Symbol::Period))
			return parse_object_member(identifier);

		// Check if it's a function call
		if (is_symbol(current_token, Symbol::ParenthesisOpen))
			return parse_function_call(identifier);

		// Check if it's a variable assignment
		if (is_operator(current_token, Operator::Assignment))
		{
			// Eat the assignment token
			expect(Operator::Assignment);

			// Parse the assigned value as an expression
			auto value = parse_expression({ Symbol::Semicolon });

			ASTNode node;
			node["type"] = ASTNodeType_VariableAssignment;
			node["name"] = identifier;
			node["value"] = value;

			expect(Symbol::Semicolon);

			return node;
		}

		ASTNode node;
		node["type"] = ASTNodeType_Identifier;
		node["value"] = identifier;

		return node;
	}

	ASTNode Parser::parse_object_member(const std::string& parent_object)
	{
		expect(Symbol::Period);

		// Get the identifier value
		auto identifier = as<IdentifierToken>(current_token)->value;
		expect(TokenType::Identifier);

		// Check for namespace presence
		while (is_operator(current_token, Operator::Namespace))
		{
			expect(Operator::Namespace);
			identifier.append("::");

			auto next_id_token = expect(TokenType::Identifier);
			identifier.append(as<IdentifierToken>(next_id_token)->value);
		}

		// Check if it's a function call
		if (is_symbol(current_token, Symbol::ParenthesisOpen))
		{
			auto function_call_node = parse_function_call(identifier);
			function_call_node["caller"] = parent_object;
			return function_call_node;
		}

		return ASTNode();
	}
	
	ASTNode Parser::parse_expression(const std::initializer_list<Symbol>& stop_symbols)
	{
		if (is_stop_symbol(current_token, stop_symbols))
		{
			throw "Expected an expression";
		}

		//
		// Expression types:
		// binary:
		//		ex. 42 + var * 56 + 7
		// 
		// single variable:
		//		ex. myVar
		//
		// literal value:
		//		ex. 56
		//
		// nested expression:
		//		ex. var * (age + 5)
		//

		// Parsing approach:
		// starting points:
		//		symbol: ( --> nested expression
		//
		//		literal: 45 --> lhs, parse for next token
		//
		//		identifier: myVar --> lhs, parse for next token, if the next
		//							  token isn't a binary operator, then
		//							  check for a symbol, if it's "(", then it's a function call,
		//							  otherwise it's a variable.

		// Prepare parsing the left hand side of the expression
		ASTNode lhs;

		// Checking if current token is an identifier
		if (current_token->type == TokenType::Identifier)
		{
			lhs = parse_identifier();
		}
		else if (is_keyword(current_token, Keyword::New))
		{
			// Check if "new" object is being allocated
			expect(Keyword::New);

			// Get the class name
			auto id_token = expect(TokenType::Identifier);
			auto class_name = as<IdentifierToken>(id_token)->value;

			// Check for namespace presence
			while (is_operator(current_token, Operator::Namespace))
			{
				expect(Operator::Namespace);
				class_name.append("::");

				auto next_id_token = expect(TokenType::Identifier);
				class_name.append(as<IdentifierToken>(next_id_token)->value);
			}

			auto node = parse_function_call(class_name);
			node["type"] = ASTNodeType_ClassInstantiation;

			return node;
		}
		else if (is_symbol(current_token, Symbol::ParenthesisOpen))
		{
			// Check if left hand side is a nested expression
			expect(Symbol::ParenthesisOpen);
			lhs = parse_expression({ Symbol::ParenthesisClose });

			expect(Symbol::ParenthesisClose);
		}
		else if (current_token->type == TokenType::LiteralValue)
		{
			// Literal value token
			auto lvt = as<LiteralValueToken>(current_token);

			lhs["type"] = ASTNodeType_Literal;
			lhs["value_type"] = LiteralValueToken::type_to_string(lvt->value_type);
			lhs["value"] = lvt->value;

			expect(TokenType::LiteralValue);
		}
		else
			return ASTNode();

		// If stop symbol is found, then return the
		// left hand side of the expression as the value.
		if (is_stop_symbol(current_token, stop_symbols))
		{
			//expect(as<SymbolToken>(current_token)->symbol);
			return lhs;
		}

		// If any other symbol is found, stop parsing the expression
		if (current_token->type == TokenType::Symbol)
			return lhs;

		// Prepare parsing the right hand side of the expression
		ASTNode rhs;

		ASTNode node;
		return node;
	}

	ASTNode Parser::parse_function_call(const std::string& fn_name)
	{
		expect(Symbol::ParenthesisOpen);

		// Parse function arguments
		json11::Json::array args;

		while (!is_symbol(current_token, Symbol::ParenthesisClose))
		{
			auto arg = parse_expression({ Symbol::Comma, Symbol::ParenthesisClose });
			args.push_back(arg);

			if (is_symbol(current_token, Symbol::ParenthesisClose))
				break;

			expect(TokenType::Symbol);
		}
		expect(Symbol::ParenthesisClose);

		ASTNode node;
		node["type"] = ASTNodeType_FunctionCall;
		node["name"] = fn_name;
		node["args"] = args;
		return node;
	}

	ASTNode Parser::parse_function_declaration()
	{
		ASTNode node;

		// Parse function name
		expect(Keyword::Func);
		auto id_token = expect(TokenType::Identifier);

		// Parse function parameters
		json11::Json::array params;

		expect(Symbol::ParenthesisOpen);
		while (!is_symbol(current_token, Symbol::ParenthesisClose))
		{
			// Get the identifier token that represents the variable name
			auto param_token = expect(TokenType::Identifier);

			// Add the variable name to the list of function parameters
			params.push_back(as<IdentifierToken>(param_token)->value);

			// If the next token is a closed parenthesis, stop looping,
			// otherwise expect a comma as a variable separator.
			if (is_symbol(current_token, Symbol::ParenthesisClose))
				break;

			expect(Symbol::Comma);
		}
		expect(Symbol::ParenthesisClose);

		// After function signature has to follow a function body,
		// function body is just a list of statements.
		json11::Json::array function_body;

		// Prepare ahead of time a return statement entry
		node[ASTNodeType_ReturnStatement] = json11::Json::NUL;

		expect(Symbol::BraceOpen);
		while (!is_symbol(current_token, Symbol::BraceClose))
		{
			auto body_node = parse_statement();

			if (!body_node.empty())
			{
				if (body_node["type"].string_value()._Equal(ASTNodeType_ReturnStatement))
					node[ASTNodeType_ReturnStatement] = body_node;
				else
					function_body.push_back(body_node);
			}

			if (is_symbol(current_token, Symbol::BraceClose))
				break;
		}
		expect(Symbol::BraceClose);

		node["type"] = ASTNodeType_FunctionDeclaration;
		node["name"] = as<IdentifierToken>(id_token)->value;
		node["params"] = params;
		node["body"] = function_body;

		return node;
	}

	ASTNode Parser::parse_variable_declaration()
	{
		// Parse the name of the variable
		expect(Keyword::Var);
		auto name_token = expect(TokenType::Identifier);
		expect(Operator::Assignment);

		// Parse value of the variable
		ASTNode node;

		node["type"] = ASTNodeType_VariableDeclaration;
		node["name"] = as<IdentifierToken>(name_token)->value;
		node["value"] = parse_expression({ Symbol::Semicolon });

		expect(Symbol::Semicolon);

		return node;
	}
}
