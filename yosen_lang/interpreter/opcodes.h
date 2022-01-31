#pragma once

namespace yosen
{
	namespace opcodes
	{
		using opcode_t = unsigned short;

		// Loads a variable into the Last Loaded Object pointer.
		// Operand: index/key of the variable in the frame's variable map.
		constexpr opcode_t LOAD			= 0x50;

		// Loads a variable into the Last Loaded Object pointer.
		// Operand: index/key of the variable in the frame's constants map.
		constexpr opcode_t LOAD_CONST	= 0x51;

		// Loads a specified parameter into the Last Loaded Object pointer.
		// Operand: index/key of the variable in the frame's parameters map.
		constexpr opcode_t LOAD_PARAM	= 0x52;

		// Copies the last loaded object into the specified variable.
		// Operand: index/key of the variable in the frame's variable map.
		constexpr opcode_t STORE		= 0x53;

		// Calls a function specified by the operands.
		// 1st Operand: index of the function's name in the frame's functions array.
		// 2nd Operand: flag that specifies whether a function has an owner object, 0x00 if a static function.
		constexpr opcode_t CALL			= 0x60;

		// Copies the last loaded object onto the stack frame's parameter stack.
		// No operands
		constexpr opcode_t PUSH			= 0x81;

		// Pops the last object from the stack frame's parameter stack.
		// No operands
		constexpr opcode_t POP			= 0x82;

		// Loads the variable from speicified register into the Last Loaded Object pointer.
		// Operand: index/key of the frame's register.
		constexpr opcode_t REG_LOAD		= 0x90;

		/*
		* Copies the last loaded object into the specified frame's registger.
		* Operand: index/key of the frame's register.
		
		* Available Registers:
			0x01 - Allocated Object Register
			0x02 - Return Register
		*/
		constexpr opcode_t REG_STORE	= 0x91;

		// Allocates a new instance of a class and stores it in the AOR (Allocated Object Register).
		// 1st Operand:	index of the class name in the frame's class names array.
		constexpr opcode_t ALLOC_OBJECT	= 0x95;

		// Imports a dynamic module or library.
		// 1st Operand:	index of the library name in the frame's imported library names array.
		constexpr opcode_t IMPORT_LIB	= 0x99;

		// Adds the last two loaded objects and stores the result into the last loaded object.
		// No operands
		constexpr opcode_t ADD = 0x30;

		// Subtracts the last two loaded objects and stores the result into the last loaded object.
		// No operands
		constexpr opcode_t SUB = 0x31;

		// Multiplies the last two loaded objects and stores the result into the last loaded object.
		// No operands
		constexpr opcode_t MUL = 0x32;

		// Divides the last two loaded objects and stores the result into the last loaded object.
		// No operands
		constexpr opcode_t DIV = 0x33;

		// Calculates the remainder of the last two loaded objects and stores the result into the last loaded object.
		// No operands
		constexpr opcode_t MOD = 0x34;

#define RETURN REG_STORE, 0x02
	}
}