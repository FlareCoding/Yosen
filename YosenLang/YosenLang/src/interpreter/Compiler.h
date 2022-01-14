#pragma once
#include "StackFrame.h"
#include "opcodes.h"

// Primitive types
#include "../native_objects/YosenString.h"
#include "../native_objects/YosenInteger.h"
#include "../native_objects/YosenTuple.h"

// Forward declaration
namespace json11 { class Json; }

namespace yosen
{
	using bytecode_t = std::vector<opcodes::opcode_t>;

	struct YosenFunction
	{
		StackFrame stack_frame;
		bytecode_t bytecode;
	};

	class Compiler
	{
	public:
		// Compiles the source code into bytecode
		void compile_source(std::string& source);

		// Check if a user-defined function with the specified name exists
		bool is_user_defined_function(const std::string& name);

		// Returns the user-defined YosenFunction object which
		// contains a stack frame and the function's bytecode.
		YosenFunction& get_user_defined_function(const std::string& name);

		// Deallocates all compiled resources
		void shutdown();

		void compile_function(const std::string& source);
		void debug_print_bytecode(bytecode_t& bytecode);

	private:
		std::map<std::string, YosenFunction> user_compiled_functions;

		// List of all compiled stack frames
		std::vector<StackFrame*> allocated_stack_frames;

		// Deallocates final resources from each created
		// stack frame making each stack frame unusable.
		void destroy_stack_frame(StackFrame& stack_frame);

		// Compiles the entire function from the AST
		// and stores it in the user_compiled_functions map.
		void compile_function(json11::Json* node_ptr);
		
		// Compiles a generic statement AST node
		void compile_statement(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		// Compiles a variable declaration AST node
		void compile_variable_declaration(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		// Compiles a variable assignment AST node
		void compile_variable_assignment(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		// Compiles a function call AST node
		void compile_function_call(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

		// Compiles a generic expression based on the AST node
		void compile_expression(json11::Json* node_ptr, StackFrame& stack_frame, bytecode_t& bytecode);

	private:
		// Returns the key for the constant defined by the AST node
		uint32_t get_constant_literal_key(json11::Json* node_ptr, StackFrame& stack_frame);

		// Returns the key for the variable defined by the AST node
		uint32_t get_variable_key(json11::Json* node_ptr, StackFrame& stack_frame);
	};
}
