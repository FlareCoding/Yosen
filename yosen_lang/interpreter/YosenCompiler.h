#pragma once
#include <YosenEnvironment.h>
#include "StackFrame.h"
#include "opcodes.h"

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

	class YosenCompiler
	{
	public:
		void debug_print_bytecode(bytecode_t& bytecode);

	private:
		// Returns the key for the constant defined by the AST node
		uint32_t get_constant_literal_key(json11::Json* node_ptr, StackFrame& stack_frame);

		// Returns the key for the variable defined by the AST node
		uint32_t get_variable_key(json11::Json* node_ptr, StackFrame& stack_frame);
	};
}
