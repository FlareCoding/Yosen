#pragma once
#include "../native_objects/YosenObject.h"
#include <vector>
#include <map>

namespace yosen
{
	// Stack frame associated with a function
	struct StackFrame
	{
		// Name of the function
		std::string name;

		// Parameters of the function
		std::vector<std::pair<std::string, YosenObject*>> params;

		// Maps the variable names to stack variable keys
		std::map<std::string, uint32_t> var_keys;

		// Maps stack variable keys to native objects
		std::map<uint32_t, YosenObject*> vars;

		// Maps the constants in their string form to their stack keys
		std::map<std::string, uint32_t> constant_keys;

		// Maps stack constant keys to native objects
		std::map<uint32_t, YosenObject*> constants;

		// Names of static and member function called within this function
		std::vector<std::string> function_names;

		// Variable assignment register
		YosenObject** assignment_register = nullptr;
	};
}
