#pragma once
#include "YosenCompiler.h"
#include <stack>

namespace yosen
{
	enum class RegisterType
	{
		AllocatedObjectRegister = 1, // stores temporary value to be assigned to a loaded object
		ReturnRegister				 // stores the return value from a function
	};


	class YosenInterpreter
	{
	public:
		YosenInterpreter();

		// Initializes the runtime environment
		void init();

		// Shuts down the runtime environment and deallocates memory
		void shutdown();

		// Creates an interactive Yosen console that accepts
		// a continuous input of source code commands.
		void run_interactive_shell();

		// Executes the entire code of the function
		void execute_bytecode(StackFrame& stack_frame, bytecode_t& bytecode);

		// Frees all objects in the stack frame
		void deallocate_frame(StackFrame& stack_frame);

		// Deallocates final resources from each created
		// stack frame making each stack frame unusable.
		void destroy_stack_frame(StackFrame& stack_frame);

	private:
		YosenEnvironment& m_env;

		// Reference to the Last Loaded Object
		YosenObject** LLOref = nullptr;

		std::map<RegisterType, YosenObject*> registers = {
			// Register to hold objects that were dynamically allocated
			{ RegisterType::AllocatedObjectRegister, nullptr },

			// Register to hold the return value of the most recent function
			{ RegisterType::ReturnRegister, nullptr },
		};

		// Each stack frame has its own parameter stack to operate with to push variables onto
		std::stack<std::vector<YosenObject*>> parameter_stacks;

	private:
		// Executes a single instruction that could consist of single or multiple opcodes.
		// Returns number of opcodes processed.
		size_t execute_instruction(StackFrame& stack_frame, opcodes::opcode_t* ops);
	};
}
