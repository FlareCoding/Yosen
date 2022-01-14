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
		// 2nd Operand: specifies the variable of the owner's object, 0x00 if a static function.
		constexpr opcode_t CALL			= 0x60;

		// Copies the last loaded object onto the frame's parameter stack.
		// No operands
		constexpr opcode_t PUSH			= 0x81;

		// Loads the variable from speicified register into the Last Loaded Object pointer.
		// Operand: index/key of the frame's register.
		constexpr opcode_t REG_LOAD		= 0x90;

		/*
		* Copies the last loaded object into the specified frame's registger.
		* Operand: index/key of the frame's register.
		
		* Available Registers:
			0x01 - Assignment Register
			0x02 - Return Register
		*/
		constexpr opcode_t REG_STORE	= 0x91;

#define RETURN REG_STORE, 0x02
	}
}
