#pragma once
#include <primitives/YosenObject.h>
#include <vector>
#include <map>

namespace yosen
{
	// Stack frame associated with a function
	class StackFrame
	{
	public:
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

		// Names of allocated classes used within the function
		std::vector<std::string> class_names;

		// Creates the needed entries for the variable
		void add_variable(const std::string& name, YosenObject* obj = nullptr);

		// Returns whether a variable exists on the stack frame
		bool has_variable(const std::string& name) const;

		// Creates a new function name entry
		void add_function_name(const std::string& name);

		// Returns the index of the function given its name,
		// returns -1 if the function doesn't exist.
		size_t get_function_index(const std::string& name);

		// Returns the index of the class name,
		// returns -1 if the name doesn't exist.
		size_t get_class_name_index(const std::string& name);
	};
}