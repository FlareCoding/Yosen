#include "Parser.h"

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

	TokenRef<Token> Parser::expect(Symbol symbol)
	{
		if (current_token->type != TokenType::Symbol)
		{
			throw "Expected a symbol";
			return current_token;
		}

		if (as<SymbolToken>(current_token)->symbol != symbol)
		{
			throw "Unexpected symbol found";
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
			throw "Expected a keyword";
			return current_token;
		}

		if (as<KeywordToken>(current_token)->keyword != keyword)
		{
			throw "Unexpected keyword found";
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
			throw "Expected an operator";
			return current_token;
		}

		if (as<OperatorToken>(current_token)->op != op)
		{
			throw "Unexpected operator found";
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
			throw "Unexpected token type";
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
	
	AST Parser::construct_AST()
	{
		AST ast;

		while (current_token)
		{
			parse_segment(ast);

			// Make sure to advance only if the current token is not nullptr (eof)
			/*if (current_token)
				current_token = next_token();*/
		}

		return ast;
	}
	
	void Parser::parse_segment(AST& ast)
	{
		if (is_keyword(current_token, Keyword::Func))
		{
			auto node = parse_function_declaration();
			ast.functions.push_back(node);
		}
	}

	ASTNode Parser::parse_statement()
	{
		if (is_keyword(current_token, Keyword::Var))
		{
			return parse_variable_declaration();
		}

		if (current_token->type == TokenType::Identifier)
		{
			return parse_identifier();
		}

		// Check if it's a semicolon, then continue onto the next token
		if (is_symbol(current_token, Symbol::Semicolon))
		{
			current_token = next_token();
			return ASTNode();
		}

		throw "Unknown statement";
		return ASTNode();
	}
	
	ASTNode Parser::parse_expression(Symbol stop_symbol)
	{
		if (is_symbol(current_token, stop_symbol))
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

		// Check if left hand side is a nested expression
		if (is_symbol(current_token, Symbol::ParenthesisOpen))
		{
			expect(Symbol::ParenthesisOpen);
			lhs = parse_expression(Symbol::ParenthesisClose);
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
		else if (current_token->type == TokenType::Identifier)
			lhs = parse_identifier();
		else
			return ASTNode();

		// If stop symbol is found, then return the
		// left hand side of the expression as the value.
		if (is_symbol(current_token, stop_symbol))
		{
			expect(stop_symbol);
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

	ASTNode Parser::parse_identifier()
	{
		auto id_token = expect(TokenType::Identifier);

		// Check if it's a function call
		if (is_symbol(current_token, Symbol::ParenthesisOpen))
		{
			expect(Symbol::ParenthesisOpen);

			// Parse function arguments
			json11::Json::array args;

			while (!is_symbol(current_token, Symbol::ParenthesisClose))
			{
				auto arg = parse_expression(Symbol::Comma);
				args.push_back(arg);
			}
			expect(Symbol::ParenthesisClose);

			ASTNode node;
			node["type"] = ASTNodeType_FunctionCall;
			node["name"] = as<IdentifierToken>(id_token)->value;
			node["args"] = args;

			return node;
		}

		// Check if a member variable/function is being accessed/called
		if (is_symbol(current_token, Symbol::Period))
		{
			expect(Symbol::Period);

			auto member_node = parse_identifier();
			member_node["caller_object"] = as<IdentifierToken>(id_token)->value;

			return member_node;
		}

		// Check if it's an assignment operation
		if (is_operator(current_token, Operator::Assignment))
		{
			// Eat the assignment token
			expect(Operator::Assignment);

			// Parse the assigned value as an expression
			auto value = parse_expression(Symbol::Semicolon);

			ASTNode node;
			node["type"] = ASTNodeType_VariableAssignment;
			node["name"] = as<IdentifierToken>(id_token)->value;
			node["value"] = value;

			return node;
		}

		ASTNode node;
		node["type"] = ASTNodeType_Identifier;
		node["value"] = as<IdentifierToken>(id_token)->value;

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

		node["type"]	= ASTNodeType_FunctionDeclaration;
		node["name"]	= as<IdentifierToken>(id_token)->value;
		node["params"]	= params;
		node["body"]	= function_body;

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
		node["value"] = parse_expression(Symbol::Semicolon);

		return node;
	}
}
