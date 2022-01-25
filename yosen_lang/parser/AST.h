#pragma once
#include <parser/json/json11.hpp>

namespace yosen::parser
{
	using ASTNode = json11::Json::object;

	class AST
	{
	public:
		std::vector<json11::Json> functions;
		std::vector<json11::Json> class_definitions;
	};

	constexpr auto ASTNodeType_FunctionDeclaration	= "function_declaration";
	constexpr auto ASTNodeType_FunctionCall			= "function_call";
	constexpr auto ASTNodeType_VariableDeclaration	= "variable_declaration";
	constexpr auto ASTNodeType_VariableAssignment	= "variable_assignment";
	constexpr auto ASTNodeType_ReturnStatement		= "return_statement";
	constexpr auto ASTNodeType_Identifier			= "identifier";
	constexpr auto ASTNodeType_Literal				= "literal";
}
