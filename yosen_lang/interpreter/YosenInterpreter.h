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
		// Initializes the runtime environment
		void init();

		// Shuts down the runtime environment and deallocates memory
		void shutdown();

		// Compiles and runs a complete string of source code.
		// ** Expects an entry point defined as "main" by default.
		void run_source(std::string& source, const std::vector<std::string>& cmd_arguments);

		// Creates an interactive Yosen console that accepts
		// a continuous input of source code commands.
		void run_interactive_shell();

		// Executes the entire code of the function
		void execute_bytecode(StackFramePtr stack_frame, bytecode_t& bytecode);

		// Frees all objects in the stack frame
		void deallocate_frame(StackFramePtr stack_frame);

		// Deallocates final resources from each created
		// stack frame making each stack frame unusable.
		void destroy_stack_frame(StackFramePtr stack_frame);

	private:
		YosenEnvironment*	m_env;
		YosenCompiler		m_compiler;

		// Specifies whether the interpreter is
		// running in the interactive console mode.
		bool m_interactive_mode = false;

		// Tells the interactive console to stop interpreting
		// the current command due to an exception occuring.
		bool m_interactive_shell_exception_occured = false;

		// Command line arguments for a full
		// program's entry point function.
		YosenObject* m_entry_point_args = nullptr;

		// Reference to the Last Loaded Object
		YosenObject** LLOref = nullptr;

		std::map<RegisterType, YosenObject*> m_registers = {
			// Register to hold objects that were dynamically allocated
			{ RegisterType::AllocatedObjectRegister, nullptr },

			// Register to hold the return value of the most recent function
			{ RegisterType::ReturnRegister, nullptr },
		};

		// Each stack frame has its own parameter stack to operate with to push variables onto
		std::stack<std::vector<YosenObject*>> m_parameter_stacks;

		// All allocated stack frames
		std::vector<StackFramePtr> m_allocated_stack_frames;

	private:
		// Executes a single instruction that could consist of single or multiple opcodes.
		// Returns number of opcodes processed.
		size_t execute_instruction(StackFramePtr stack_frame, opcodes::opcode_t* ops);

		// Used within the interactive shell to parse function declarations
		std::string read_block_source(const std::string& header, const std::string& tab_space);

	private:
		// Main exception handler
		void main_exception_handler(const YosenException& ex);
	};
}
