#pragma once
#include <YosenEnvironment.h>
#include "opcodes.h"
#include <stack>

// Forward declaration
namespace json11 { class Json; }

namespace yosen
{
	using bytecode_t = std::vector<opcodes::opcode_t>;

	struct ProgramSource
	{
		std::vector<ys_runtime_function_t> runtime_functions;
	};

	class YosenCompiler
	{
	public:
		// Prints teh bytecode in a form of readable instructions
		void debug_print_bytecode(bytecode_t& bytecode);

		// Compiles a string of source code into
		// a complete program source object.
		ProgramSource compile_source(std::string& source);

		// Compiles a single statement
		bytecode_t compile_single_statement(std::string& source, StackFramePtr stack_frame);

	private:
		// Returns the key for the constant defined by the AST node
		uint32_t get_constant_literal_key(json11::Json* node_ptr, StackFramePtr stack_frame);

		// Returns the key for the variable defined by the AST node
		uint32_t get_variable_key(json11::Json* node_ptr, StackFramePtr stack_frame);

		// Compiles a generic statement AST node
		void compile_statement(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

		// Compiles an import statement from the AST node
		void compile_import_statement(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

		// Compiles a generic expression based on the AST node
		void compile_expression(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

		// Compiles a function call AST node
		void compile_function_call(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

		// Compiles a function declaration from the AST
		void compile_function_declaration(json11::Json* node_ptr, ProgramSource& program_source);

		// Compiles a variable declaration AST node
		void compile_variable_declaration(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

		// Compiles a variable assignment AST node
		void compile_variable_assignment(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

		// Compiles a class instantiation logic from an AST node
		void compile_class_instantiation(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

		// Compiles a conditional if/else statement from an AST node
		void compile_conditional(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

		// Compiles a while loop from an AST node
		void compile_while_loop(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

		// Compiles a break statement from an AST node
		void compile_break_statement(json11::Json* node_ptr, StackFramePtr stack_frame, bytecode_t& bytecode);

	private:
		// In case an exception occurs, the variables
		// on the stack frame should get freed.
		void destroy_stack_frame(StackFramePtr stack_frame);
		
		// Frees all compiled resources on allocated
		// stack frames in an event if exception occurs.
		void __ys_free_compiled_resources(StackFramePtr faulty_stack_frame);

	private:
		// Each loop gets its own list of indices to be replaced.
		// If a break statement occurs, the instruction pointer should
		// jump to the end of the loop.
		std::stack<std::vector<size_t>> loop_break_jmp_operand_indices;
	};
}
