#pragma once
#include <YosenEnvironment.h>
#include "StackFrame.h"
#include "opcodes.h"

// Forward declaration
namespace json11 { class Json; }

namespace yosen
{
	using bytecode_t = std::vector<opcodes::opcode_t>;

	class YosenCompiler
	{
	public:
		// Prints teh bytecode in a form of readable instructions
		void debug_print_bytecode(bytecode_t& bytecode);

		// Compiles a string of source code.
		// Returns the prepared stack frame object.
		std::pair<StackFrame, bytecode_t> compile_source(std::string& source);

		// Compiles a single statement
		bytecode_t compile_single_statement(std::string& source, StackFrame& stack_frame);

	private:
		// Returns the key for the constant defined by the AST node
		uint32_t get_constant_literal_key(json11::Json* node_ptr, StackFrame& stack_frame);

		// Returns the key for the variable defined by the AST node
		uint32_t get_variable_key(json11::Json* node_ptr, StackFrame& stack_frame);

		// Compiles a generic statement AST node
		void compile_statement(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		// Compiles an import statement from the AST node
		void compile_import_statement(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		// Compiles a generic expression based on the AST node
		void compile_expression(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		// Compiles a function call AST node
		void compile_function_call(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		// Compiles a variable declaration AST node
		void compile_variable_declaration(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		// Compiles a variable assignment AST node
		void compile_variable_assignment(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		void compile_class_instantiation(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);
	};
}
