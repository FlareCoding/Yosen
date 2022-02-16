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
		case Operator::Mod:
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
		case Operator::Or:
		case Operator::And:
		case Operator::Not:
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
			auto ex_reason = "Line " + std::to_string(current_token->lineno)
							+ " - unexpected symbol found, expected a \""
							+ SymbolToken::symbol_to_string(symbol) + "\"";

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
			auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - expected token: " + Token::token_type_to_string(type);
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
		{
			auto import_node = parse_import_statement();
			expect(Symbol::Semicolon);
			return import_node;
		}

		if (is_keyword(current_token, Keyword::Func))
			return parse_function_declaration();

		if (is_keyword(current_token, Keyword::Class))
			return parse_class_declaration();

		return parse_statement();
	}

	ASTNode Parser::parse_statement()
	{
		ASTNode node;

		if (is_keyword(current_token, Keyword::Import))
			node = parse_import_statement();

		else if (is_keyword(current_token, Keyword::Var))
			node = parse_variable_declaration();

		else if (is_keyword(current_token, Keyword::Return))
			node = parse_return_statement();

		else if (current_token->type == TokenType::Identifier)
			node = parse_identifier();

		else if (is_keyword(current_token, Keyword::If))
			return parse_conditional_statement(); // no semicolon expected after a conditional statement

		else if (is_keyword(current_token, Keyword::While))
			return parse_while_loop();

		else if (is_keyword(current_token, Keyword::For))
			return parse_for_loop();

		else if (is_keyword(current_token, Keyword::Break))
		{
			expect(Keyword::Break);
			node["type"] = ASTNodeType_BreakStatement;
		}

		// Expecting a semicolon after a statement
		expect(Symbol::Semicolon);

		return node;
	}

	ASTNode Parser::parse_import_statement()
	{
		expect(Keyword::Import);

		std::string lib_name = "";

		// Get the imported library name
		if (current_token->type == TokenType::Identifier)
		{
			lib_name = as<IdentifierToken>(current_token)->value;
			expect(TokenType::Identifier);
		}
		else
		{
			lib_name = as<LiteralValueToken>(current_token)->value;
			expect(TokenType::LiteralValue);
		}

		ASTNode node;
		node["type"] = ASTNodeType_Import;
		node["name"] = lib_name;

		return node;
	}

	ASTNode Parser::parse_return_statement()
	{
		expect(Keyword::Return);

		ASTNode node;
		node["type"] = ASTNodeType_ReturnStatement;
		node["value"] = json11::Json::NUL;

		// Check for a void return
		if (is_symbol(current_token, Symbol::Semicolon))
			return node;

		// Parse the return value expression
		node["value"] = parse_expression({ Symbol::Semicolon });

		return node;
	}
	
	ASTNode Parser::parse_identifier(ASTNode parent)
	{
		// Get the identifier value
		auto id_token = expect(TokenType::Identifier);
		auto identifier = as<IdentifierToken>(id_token)->value;

		ASTNode node;
		node["type"] = ASTNodeType_Identifier;
		node["value"] = identifier;
		if (!parent.empty())
			node["parent"] = parent;

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
		{
			expect(Symbol::Period);
			node = parse_identifier(node);
		}

		// Check if it's a function call
		if (is_symbol(current_token, Symbol::ParenthesisOpen))
		{
			auto function_call_node = parse_function_call(identifier);
			if (!parent.empty())
				function_call_node["parent"] = parent;

			// Check for a sequenced function call statement
			if (is_symbol(current_token, Symbol::Period))
			{
				expect(Symbol::Period);
				auto next_node = parse_expression({ Symbol::Semicolon });

				if (next_node["type"].string_value() != ASTNodeType_FunctionCall &&
					next_node["type"].string_value() != ASTNodeType_NodeSequence)
				{
					auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - expected a sequence of function calls";
					YosenEnvironment::get().throw_exception(ParserException(ex_reason));
					return node;
				}

				// Adjusting the caller of the second function
				if (next_node["type"].string_value() == ASTNodeType_FunctionCall)
					next_node["parent"] = ASTCallerID_LLO;
				else if (next_node["type"].string_value() == ASTNodeType_NodeSequence)
				{
					auto first_node = next_node["first"].object_items();
					first_node["parent"] = ASTCallerID_LLO;

					next_node["first"] = first_node;

					std::function<void(ASTNode*)> fix_sequence_second_nodes = [&](ASTNode* node_ptr) {
						auto& node = *node_ptr;

						auto first_node = node["first"].object_items();
						first_node["parent"] = ASTCallerID_LLO;

						node["first"] = first_node;

						// Base case
						if (node["type"].string_value() == ASTNodeType_FunctionCall)
						{
							node["parent"] = ASTCallerID_LLO;
							return;
						}
						else if (node["type"].string_value() == ASTNodeType_NodeSequence)
						{
							const ASTNode& second_node = node["second"].object_items();
							fix_sequence_second_nodes(&const_cast<ASTNode&>(second_node));
						}
					};

					fix_sequence_second_nodes(&next_node);
				}

				ASTNode sequence_node;
				sequence_node["type"] = ASTNodeType_NodeSequence;
				sequence_node["first"] = function_call_node;
				sequence_node["second"] = next_node;

				return sequence_node;
			}

			return function_call_node;
		}

		// Check if it's a variable assignment
		if (is_operator(current_token, Operator::Assignment))
		{
			// Eat the assignment token
			expect(Operator::Assignment);

			// Parse the assigned value as an expression
			auto value = parse_expression({ Symbol::Semicolon });

			node["type"] = ASTNodeType_VariableAssignment;
			node["name"] = identifier;
			node["value"] = value;

			return node;
		}

		// Check for compact binary assignment
		if (is_operator(current_token, Operator::AdditionAssignment) ||
			is_operator(current_token, Operator::SubtractionAssignment) ||
			is_operator(current_token, Operator::MultiplicationAssignment) ||
			is_operator(current_token, Operator::DivisionAssignment))
		{
			auto op_token = as<OperatorToken>(current_token);
			expect(op_token->op);

			auto expanded_operator = "+";
			if (op_token->op == Operator::SubtractionAssignment)
				expanded_operator = "-";
			else if (op_token->op == Operator::MultiplicationAssignment)
				expanded_operator = "*";
			else if (op_token->op == Operator::DivisionAssignment)
				expanded_operator = "/";

			// Parse the right hand side
			auto rhs = parse_expression({ Symbol::Semicolon, Symbol::ParenthesisClose });

			ASTNode id_node;
			id_node["type"] = ASTNodeType_Identifier;
			id_node["value"] = identifier;

			ASTNode addition_node;
			addition_node["type"] = ASTNodeType_BinaryOperation;
			addition_node["operator"] = expanded_operator;
			addition_node["lhs"] = id_node;
			addition_node["rhs"] = rhs;

			ASTNode assignment_node;
			assignment_node["type"] = ASTNodeType_VariableAssignment;
			assignment_node["name"] = identifier;
			assignment_node["value"] = addition_node;

			return assignment_node;
		}

		return node;
	}
	
	ASTNode Parser::parse_expression(const std::initializer_list<Symbol>& stop_symbols)
	{
		if (is_stop_symbol(current_token, stop_symbols))
		{
			// Empty expression
			return ASTNode();
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
		else if (is_operator(current_token, Operator::Not))
		{
			// Checking if a boolean "!" operator has been applied
			expect(Operator::Not);

			//
			// Parse the right hand side value of the not operator
			//
			// The right hand side could be an identifier/function-call or a nested expression
			if (is_symbol(current_token, Symbol::ParenthesisOpen))
			{
				// Expect this to be a nested expression
				expect(Symbol::ParenthesisOpen);
				auto conditional = parse_expression({ Symbol::ParenthesisClose });
				expect(Symbol::ParenthesisClose);

				ASTNode literal_value_node;
				literal_value_node["type"] = ASTNodeType_Literal;
				literal_value_node["value_type"] = LiteralValueToken::type_to_string(LiteralType::Boolean);
				literal_value_node["value"] = "true";

				lhs["type"] = ASTNodeType_BooleanOperation;
				lhs["operator"] = "!=";
				lhs["lhs"] = conditional;
				lhs["rhs"] = literal_value_node;
			}
			else
			{
				// In any other case, expect an identifier
				auto conditional = parse_identifier();

				ASTNode literal_value_node;
				literal_value_node["type"] = ASTNodeType_Literal;
				literal_value_node["value_type"] = LiteralValueToken::type_to_string(LiteralType::Boolean);
				literal_value_node["value"] = "true";

				lhs["type"] = ASTNodeType_BooleanOperation;
				lhs["operator"] = "!=";
				lhs["lhs"] = conditional;
				lhs["rhs"] = literal_value_node;
			}
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

			// Check for a sequenced function call statement
			if (is_symbol(current_token, Symbol::Period))
			{
				expect(Symbol::Period);
				auto next_node = parse_expression(stop_symbols);

				if (next_node["type"].string_value() != ASTNodeType_FunctionCall &&
					next_node["type"].string_value() != ASTNodeType_NodeSequence)
				{
					auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - expected a sequence of function calls";
					YosenEnvironment::get().throw_exception(ParserException(ex_reason));
					return node;
				}

				// Adjusting the caller of the second function
				if (next_node["type"].string_value() == ASTNodeType_FunctionCall)
					next_node["parent"] = ASTCallerID_LLO;
				else if (next_node["type"].string_value() == ASTNodeType_NodeSequence)
				{
					auto first_node = next_node["first"].object_items();
					first_node["parent"] = ASTCallerID_LLO;

					next_node["first"] = first_node;

					std::function<void(ASTNode*)> fix_sequence_second_nodes = [&](ASTNode* node_ptr) {
						auto& node = *node_ptr;

						auto first_node = node["first"].object_items();
						first_node["parent"] = ASTCallerID_LLO;

						node["first"] = first_node;

						// Base case
						if (node["type"].string_value() == ASTNodeType_FunctionCall)
						{
							node["parent"] = ASTCallerID_LLO;
							return;
						}
						else if (node["type"].string_value() == ASTNodeType_NodeSequence)
						{
							const ASTNode& second_node = node["second"].object_items();
							fix_sequence_second_nodes(&const_cast<ASTNode&>(second_node));
						}
					};

					fix_sequence_second_nodes(&next_node);
				}

				ASTNode sequence_node;
				sequence_node["type"] = ASTNodeType_NodeSequence;
				sequence_node["first"] = node;
				sequence_node["second"] = next_node;

				return sequence_node;
			}

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

			// If the value is a string, wrap it with an extra
			// set of quotes to work properly on the stack frame.
			if (lvt->value_type == LiteralType::String)
				lhs["value"] = "\"" + lvt->value + "\"";

			expect(TokenType::LiteralValue);
		}
		else if (is_symbol(current_token, Symbol::BracketOpen))
		{
			// List instantiation
			expect(Symbol::BracketOpen);

			json11::Json::array list_items;

			while (!is_symbol(current_token, Symbol::BracketClose))
			{
				auto elem_node = parse_expression({ Symbol::Comma, Symbol::BracketClose });
				list_items.push_back(elem_node);

				// If the next token is a closed parenthesis, stop looping,
				// otherwise expect a comma as a variable separator.
				if (is_symbol(current_token, Symbol::BracketClose))
					break;

				expect(Symbol::Comma);
			}

			expect(Symbol::BracketClose);

			std::string list_init_value;
			for (auto& item : list_items)
				if (item["value"] != json11::Json::NUL)
					list_init_value.append(item["value"].string_value() + ",");

			lhs["type"] = ASTNodeType_Literal;
			lhs["value_type"] = "list";
			
			if (!list_init_value.empty())
				lhs["value"] = list_init_value.substr(0, list_init_value.size() - 1); // remove last comma

			return lhs;
		}

		// If stop symbol is found, then return the
		// left hand side of the expression as the value.
		if (is_stop_symbol(current_token, stop_symbols))
		{
			return lhs;
		}

		//
		// Checking if there is a right side of the expression
		// in the case of a binary or boolean operator usage.
		//
		if (current_token->type == TokenType::Operator)
		{
			auto op_token = as<OperatorToken>(current_token);
			auto op = op_token->op;

			// Make sure the token is either one of binary or boolean operators
			if (!IsBinaryOperator(op) && !IsBooleanOperator(op))
			{
				auto ex_reason = "Line " + std::to_string(current_token->lineno) + " - unexpected operator found: " + op_token->value;
				YosenEnvironment::get().throw_exception(ParserException(ex_reason));
			}

			// Consume the operator token
			expect(op);

			// Determine the operation type
			auto node_type = IsBinaryOperator(op) ? ASTNodeType_BinaryOperation : ASTNodeType_BooleanOperation;

			// Parse the right hand side of the expression
			ASTNode rhs = parse_expression(stop_symbols);

			ASTNode result_node;
			result_node["type"]		= node_type;
			result_node["operator"]	= op_token->value;
			result_node["lhs"]		= lhs;
			result_node["rhs"]		= rhs;

			// If the operator is <= or >= the expression
			// has to be expanded into ((x < y) || (x == y)).
			if (op_token->op == Operator::GreaterThanOrEqual ||
				op_token->op == Operator::LessThanOrEqual)
			{
				auto left_operator = 
					(op_token->op == Operator::GreaterThanOrEqual)
					? ">"
					: "<";

				// Making the overall expression an OR expression
				result_node["operator"] = "||";

				ASTNode lhs_calculation;
				lhs_calculation["type"]		= ASTNodeType_BooleanOperation;
				lhs_calculation["operator"] = left_operator;
				lhs_calculation["lhs"]		= lhs;
				lhs_calculation["rhs"]		= rhs;

				ASTNode rhs_calculation;
				rhs_calculation["type"]		= ASTNodeType_BooleanOperation;
				rhs_calculation["operator"] = "==";
				rhs_calculation["lhs"]		= lhs;
				rhs_calculation["rhs"]		= rhs;

				// Adjusting the result node
				result_node["lhs"] = lhs_calculation;
				result_node["rhs"] = rhs_calculation;
			}

			return result_node;
		}

		return lhs;
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

			expect(Symbol::Comma);
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

		return node;
	}
	
	ASTNode Parser::parse_conditional_statement()
	{
		// First if statement
		expect(Keyword::If);

		// Beginning of a condition expression
		expect(Symbol::ParenthesisOpen);
		
		// Parse the condition expression
		auto condition = parse_expression({ Symbol::ParenthesisClose });

		// End of the condition expression
		expect(Symbol::ParenthesisClose);

		// List of all statements in the "if" statement's body
		json11::Json::array if_body_statements;

		// Start processing body statements
		expect(Symbol::BraceOpen);
		while (!is_symbol(current_token, Symbol::BraceClose))
		{
			auto body_node = parse_statement();

			if (!body_node.empty())
				if_body_statements.push_back(body_node);
		}
		expect(Symbol::BraceClose);

		// List of all statements in the "if" statement's body
		json11::Json::array else_body_statements;

		if (is_keyword(current_token, Keyword::Else))
		{
			expect(Keyword::Else);

			// Check if it's an "else if" statement
			if (is_keyword(current_token, Keyword::If))
			{
				auto else_body_node = parse_conditional_statement();
				else_body_statements.push_back(else_body_node);
			}
			else
			{
				// Start processing body statements
				expect(Symbol::BraceOpen);
				while (!is_symbol(current_token, Symbol::BraceClose))
				{
					auto body_node = parse_statement();

					if (!body_node.empty())
						else_body_statements.push_back(body_node);
				}
				expect(Symbol::BraceClose);
			}
		}

		ASTNode result;
		result["type"] = ASTNodeType_Conditional;
		result["condition"] = condition;
		result["if_body"] = if_body_statements;
		
		if (else_body_statements.size())
			result["else_body"] = else_body_statements;

		return result;
	}

	ASTNode Parser::parse_while_loop()
	{
		expect(Keyword::While);

		// Beginning of a condition expression
		expect(Symbol::ParenthesisOpen);

		// Parse the condition expression
		auto condition = parse_expression({ Symbol::ParenthesisClose });

		// End of the condition expression
		expect(Symbol::ParenthesisClose);

		// List of all statements in the loop's body
		json11::Json::array loop_body_statements;

		// Start processing body statements
		expect(Symbol::BraceOpen);
		while (!is_symbol(current_token, Symbol::BraceClose))
		{
			auto body_node = parse_statement();

			if (!body_node.empty())
				loop_body_statements.push_back(body_node);
		}
		expect(Symbol::BraceClose);

		ASTNode result;
		result["type"] = ASTNodeType_WhileLoop;
		result["condition"] = condition;
		result["body"] = loop_body_statements;

		return result;
	}
	
	ASTNode Parser::parse_for_loop()
	{
		expect(Keyword::For);

		// Beginning of a for loop conditions
		expect(Symbol::ParenthesisOpen);

		// Parse the initial statement
		auto init_statement = parse_statement();

		// Parse the condition expression
		auto condition = parse_expression({ Symbol::Semicolon });
		expect(Symbol::Semicolon);

		// Parse the expression that will be executed after each iteration
		auto post_iteration_statement = parse_expression({ Symbol::ParenthesisClose });

		// End of for loop conditions
		expect(Symbol::ParenthesisClose);

		// List of all statements in the loop's body
		json11::Json::array loop_body_statements;

		// Start processing body statements
		expect(Symbol::BraceOpen);
		while (!is_symbol(current_token, Symbol::BraceClose))
		{
			auto body_node = parse_statement();

			if (!body_node.empty())
				loop_body_statements.push_back(body_node);
		}
		expect(Symbol::BraceClose);

		ASTNode result;
		result["type"]			 = ASTNodeType_ForLoop;
		result["init_statement"] = init_statement;
		result["condition"]		 = condition;
		result["post_iteration"] = post_iteration_statement;
		result["body"]			 = loop_body_statements;

		return result;
	}
	
	ASTNode Parser::parse_class_declaration()
	{
		expect(Keyword::Class);

		// Parse the class name
		auto class_name = as<IdentifierToken>(expect(TokenType::Identifier))->value;

		// Start processing class body
		json11::Json::array class_body_nodes;

		expect(Symbol::BraceOpen);
		while (!is_symbol(current_token, Symbol::BraceClose))
		{
			if (current_token->type == TokenType::EOFToken) break;

			// Parse class body node
			auto node = parse_block();
			
			if (!node.empty())
			{
				// If it is a static function, name should
				// be changed be prepended by the class name.
				if (node["type"].string_value() == ASTNodeType_FunctionDeclaration &&
					node["params"].array_items().size() == 0 ||
					(node["params"].array_items().size() && node["params"].array_items().at(0) != "self"))
				{
					node["name"] = class_name + "::" + node["name"].string_value();
				}

				class_body_nodes.push_back(node);
			}
		}
		expect(Symbol::BraceClose);

		ASTNode result;
		result["type"] = ASTNodeType_ClassDeclaration;
		result["name"] = class_name;
		result["body"] = class_body_nodes;

		return result;
	}
}
