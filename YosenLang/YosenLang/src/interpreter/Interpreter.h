#pragma once
#include "Compiler.h"
#include <stack>

namespace yosen
{
	enum class RegisterType
	{
		AssignmentRegister = 1, // stores temporary value to be assigned to a loaded object
		ReturnRegister			// stores the return value from a function
	};

	class Interpreter
	{
	public:
		// Initializes the runtime environment
		void init();

		// Shuts down the runtime environment and deallocates memory
		void shutdown();

		// Compiles the source code string into bytecode and executes it.
		// Entry point of execution is defined by the main function.
		void run(std::string& source);

		// Executes the entire code of the function
		void execute_function(StackFrame& frame, bytecode_t& bytecode);

		// Registers a function to be usable in the runtime environment
		void register_native_runtime_function(const std::string& name, yosen_function_t fn);

	private:
		// Compiler object that compiles source code to bytecode
		Compiler compiler;

		// Path of the executable
		YosenString* executable_path_obj = nullptr;

		// Executes a single instruction that could consist of single or multiple opcodes.
		// Returns number of opcodes processed.
		size_t execute_instruction(StackFrame& frame, opcodes::opcode_t* ops);

		// Frees all objects in the stack frame
		void deallocate_frame(StackFrame& stack_frame);

		// Reference to the Last Loaded Object
		YosenObject** LLOref = nullptr;

		std::map<RegisterType, YosenObject*> registers = {
			// Register to hold objects that will get assigned to the LLO
			{ RegisterType::AssignmentRegister, nullptr },

			// Register to hold the return value of the most recent function
			{ RegisterType::ReturnRegister, nullptr },
		};

		// Each stack frame has its own parameter stack to operate with to push variables onto
		std::stack<std::vector<YosenObject*>> parameter_stacks;

		// Maps the names of native functions to their function objects
		std::map<std::string, yosen_function_t> native_runtime_functions;

	private:
		uint64_t total_allocated_objects = 0;
	};
}
