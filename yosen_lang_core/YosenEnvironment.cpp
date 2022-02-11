#include "YosenEnvironment.h"

namespace yosen
{
	static std::unique_ptr<YosenEnvironment> s_env_instance = nullptr;

	void YosenEnvironment::init()
	{
		// Create environment
		if (!s_env_instance)
			s_env_instance = std::make_unique<YosenEnvironment>();
		else
		{
			throw "Yosen environment already initialized!";
			return;
		}

		// Initialize default objects
		YosenObject_Null = allocate_object<YosenObject>();
		YosenObject_Null->m_string_repr = "null";

		// Initialize macro functions such as "typeof()"
		s_env_instance->initialize_macro_functions();

		// Initialize casting to primitive types
		s_env_instance->initialize_primitive_casting_functions();

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
		utils::log_colored(
			utils::ConsoleColor::Green,
			"[*] Yosen Environment Initialized\n"
		);
#endif
	}

	void YosenEnvironment::shutdown()
	{
		// Free all stack frames from the class builder objects
		for (auto& [name, builder] : m_runtime_class_builder_objects)
		{
			// Iterate over runtime function objects
			for (auto& [name, fn] : builder->runtime_functions)
			{
				auto& stack_frame = fn.first;

				// Deallocate constants
				for (auto& [key, obj] : stack_frame->constants)
					if (obj) free_object(obj);

				stack_frame->constants.clear();

				// Deallocate variables
				for (auto& [key, obj] : stack_frame->vars)
					if (obj) free_object(obj);

				stack_frame->vars.clear();
				stack_frame->params.clear();
			}

			for (auto& [name, var] : builder->member_variables)
				free_object(var);
		}

		// Free all the global variables
		for (auto& [name, key_obj_pair] : m_global_variable_objects)
			free_object(key_obj_pair.second);

		free_object(YosenObject_Null);
	}

	YosenEnvironment& YosenEnvironment::get()
	{
		return *s_env_instance.get();
	}

	void YosenEnvironment::register_static_native_function(const std::string& name, ys_static_native_fn_t fn)
	{
		// Apply current namespace name to the function's name
		auto full_name = m_current_module_namespace.empty() ? name : m_current_module_namespace + "::" + name;

		// Register the function
		m_static_native_functions[full_name] = fn;
	}

	bool YosenEnvironment::is_static_native_function(const std::string& name)
	{
		return m_static_native_functions.find(name) != m_static_native_functions.end();
	}

	ys_static_native_fn_t YosenEnvironment::get_static_native_function(const std::string& name)
	{
		if (is_static_native_function(name))
			return m_static_native_functions.at(name);
		else
		{
			printf("Function '%s' not found\n", name.c_str());
			return nullptr;
		}
	}

	void YosenEnvironment::register_static_runtime_function(const std::string& name, ys_runtime_function_t fn)
	{
		// Apply current namespace name to the function's name
		auto full_name = m_current_module_namespace.empty() ? name : m_current_module_namespace + "::" + name;

		// Register the function
		m_static_runtime_functions[full_name] = fn;
	}

	bool YosenEnvironment::is_static_runtime_function(const std::string& name)
	{
		return m_static_runtime_functions.find(name) != m_static_runtime_functions.end();
	}

	ys_runtime_function_t YosenEnvironment::get_static_runtime_function(const std::string& name)
	{
		if (is_static_runtime_function(name))
			return m_static_runtime_functions.at(name);
		else
		{
			printf("Runtime function '%s' not found\n", name.c_str());
			return { nullptr, std::vector<unsigned short>() };
		}
	}
	
	bool YosenEnvironment::load_yosen_module(const std::string& name)
	{
		// Load module library
		void* lib = utils::load_library(name.c_str());
		if (!lib)
		{
			utils::log_colored(utils::ConsoleColor::Red, "[*] Error: ");
			printf("Failed to find module %s\n", name.c_str());
			return false;
		}

		typedef void(*ys_module_init_fn_t)();

		// Find the module init entry point
		auto init_fn = (ys_module_init_fn_t)utils::get_proc_address(lib, "_ys_init_module");
		if (!init_fn)
		{
			utils::log_colored(utils::ConsoleColor::Red, "[*] Error: ");
			printf("Failed to load module %s, no init entry point found\n", name.c_str());
			return false;
		}

		// Initialize module
		init_fn();

		return true;
	}
	
	void YosenEnvironment::register_yosen_class(const std::string& name, ys_class_builder_fn_t builder_fn)
	{
		// Apply namespace name to the class name
		auto full_name = m_current_module_namespace.empty() ? name : m_current_module_namespace + "::" + name;

		// Register the class
		m_custom_class_builders[full_name] = builder_fn;
	}
	
	bool YosenEnvironment::is_class_name(const std::string& name)
	{
		return m_custom_class_builders.find(name) != m_custom_class_builders.end();
	}

	YosenObject* YosenEnvironment::construct_class_instance(const std::string& name, YosenObject* args)
	{
		if (m_custom_class_builders.find(name) != m_custom_class_builders.end())
			return (m_custom_class_builders.at(name))(args);
		else
		{
			printf("Class not found for '%s'\n", name.c_str());
			return YosenObject_Null->clone();
		}
	}

	std::shared_ptr<RuntimeClassBuilder> YosenEnvironment::create_runtime_class_builder(const std::string& name)
	{
		auto builder = std::make_shared<RuntimeClassBuilder>();
		builder->class_name = name;

		m_runtime_class_builder_objects[name] = builder;

		return builder;
	}

	void YosenEnvironment::register_global_variable(const std::string& name, YosenObject* value)
	{
		if (!is_global_variable(name))
			m_global_variable_objects.insert({name, { m_global_variable_objects.size(), value }});
	}

	bool YosenEnvironment::is_global_variable(const std::string& name)
	{
		return m_global_variable_objects.find(name) != m_global_variable_objects.end();
	}

	YosenObject*& YosenEnvironment::get_global_variable(const std::string& name)
	{
		return m_global_variable_objects.at(name).second;
	}

	YosenObject*& YosenEnvironment::get_global_variable(uint32_t key)
	{
		for (auto& [name, key_obj_pair] : m_global_variable_objects)
			if (key_obj_pair.first == key)
				return key_obj_pair.second;

		return YosenObject_Null;
	}

	uint32_t YosenEnvironment::get_global_variable_index(const std::string& name)
	{
		return m_global_variable_objects.at(name).first;
	}

	void YosenEnvironment::set_global_variable(const std::string& name, YosenObject* value)
	{
		if (is_global_variable(name))
			m_global_variable_objects[name].second = value;
	}

	void YosenEnvironment::set_global_variable(uint32_t key, YosenObject* value)
	{
		for (auto& [name, key_obj_pair] : m_global_variable_objects)
			if (key_obj_pair.first == key)
				key_obj_pair.second = value;
	}

	void YosenEnvironment::start_module_namespace(const std::string& name)
	{
		if (m_current_module_namespace.empty())
			m_current_module_namespace.append(name);
		else
			m_current_module_namespace.append("::" + name);
	}

	void YosenEnvironment::end_module_namespace()
	{
		auto idx = m_current_module_namespace.rfind("::");
		if (idx != std::string::npos)
			m_current_module_namespace = m_current_module_namespace.substr(0, idx);
		else
			m_current_module_namespace.clear();
	}

	void YosenEnvironment::register_exception_handler(exception_handler_t handler)
	{
		m_exception_handlers.insert(m_exception_handlers.begin(), handler);
	}

	void YosenEnvironment::throw_exception(const YosenException& ex)
	{
		for (auto& listener : m_exception_handlers)
			listener(ex);
	}
	
	void YosenEnvironment::throw_exception(const std::string& reason)
	{
		throw_exception(YosenException(reason));
	}
	
	void YosenEnvironment::initialize_primitive_casting_functions()
	{
		register_static_native_function("int", [](YosenObject* args) -> YosenObject* {
			YosenObject* arg_object = nullptr;
			arg_parse(args, "o", &arg_object);

			if (!arg_object)
				return YosenObject_Null->clone();

			auto arg_type = arg_object->runtime_name();

			if (strcmp(arg_type, "Integer") == 0)
			{
				return allocate_object<YosenInteger>(static_cast<YosenInteger*>(arg_object)->value);
			}
			else if (strcmp(arg_type, "Float") == 0)
			{
				return allocate_object<YosenInteger>((int64_t)static_cast<YosenFloat*>(arg_object)->value);
			}
			else if (strcmp(arg_type, "String") == 0)
			{
				int64_t val = 0;
				try {
					val = (int64_t)std::stoi(static_cast<YosenString*>(arg_object)->value);
				}
				catch (...) {
					return YosenObject_Null->clone();
				}

				return allocate_object<YosenInteger>(val);
			}
			else if (strcmp(arg_type, "Boolean") == 0)
			{
				return allocate_object<YosenInteger>((int64_t)static_cast<YosenBoolean*>(arg_object)->value);
			}

			return YosenObject_Null->clone();
		});

		register_static_native_function("float", [](YosenObject* args) -> YosenObject* {
			YosenObject* arg_object = nullptr;
			arg_parse(args, "o", &arg_object);

			if (!arg_object)
				return YosenObject_Null->clone();

			auto arg_type = arg_object->runtime_name();

			if (strcmp(arg_type, "Float") == 0)
			{
				return allocate_object<YosenFloat>(static_cast<YosenFloat*>(arg_object)->value);
			}
			else if (strcmp(arg_type, "Integer") == 0)
			{
				return allocate_object<YosenFloat>((double)static_cast<YosenInteger*>(arg_object)->value);
			}
			else if (strcmp(arg_type, "String") == 0)
			{
				double val = 0;
				try {
					val = std::stod(static_cast<YosenString*>(arg_object)->value);
				}
				catch (...) {
					return YosenObject_Null->clone();
				}

				return allocate_object<YosenFloat>(val);
			}
			else if (strcmp(arg_type, "Boolean") == 0)
			{
				return allocate_object<YosenFloat>((double)static_cast<YosenBoolean*>(arg_object)->value);
			}
			
			return YosenObject_Null->clone();
		});

		register_static_native_function("bool", [](YosenObject* args) -> YosenObject* {
			YosenObject* arg_object = nullptr;
			arg_parse(args, "o", &arg_object);

			if (!arg_object)
				return YosenObject_Null->clone();

			auto arg_type = arg_object->runtime_name();

			if (strcmp(arg_type, "Boolean") == 0)
			{
				return allocate_object<YosenBoolean>(static_cast<YosenBoolean*>(arg_object)->value);
			}
			else if (strcmp(arg_type, "Integer") == 0)
			{
				return allocate_object<YosenBoolean>((bool)static_cast<YosenInteger*>(arg_object)->value);
			}
			else if (strcmp(arg_type, "Float") == 0)
			{
				return allocate_object<YosenBoolean>((bool)static_cast<YosenFloat*>(arg_object)->value);
			}
			else if (strcmp(arg_type, "String") == 0)
			{
				bool val = false;
				auto str_val = static_cast<YosenString*>(arg_object)->value;

				if (str_val == "true")
					val = true;
				else if (str_val == "false")
					val = false;
				else
				{
					return YosenObject_Null->clone();
				}

				return allocate_object<YosenFloat>(val);
			}

			return YosenObject_Null->clone();
		});

		register_static_native_function("str", [](YosenObject* args) -> YosenObject* {
			YosenObject* arg_object = nullptr;
			arg_parse(args, "o", &arg_object);

			if (!arg_object)
				return YosenObject_Null->clone();

			return allocate_object<YosenString>(arg_object->to_string());
		});
	}
	
	void YosenEnvironment::initialize_macro_functions()
	{
		register_static_native_function("typeof", [](YosenObject* args) -> YosenObject* {
			YosenObject* arg_object = nullptr;
			arg_parse(args, "o", &arg_object);

			if (!arg_object)
				return YosenObject_Null->clone();

			return allocate_object<YosenString>(arg_object->runtime_name());
		});

		register_static_native_function("instanceof", [](YosenObject* args) -> YosenObject* {
			YosenObject* arg_object = nullptr;
			arg_parse(args, "o", &arg_object);

			if (!arg_object)
				return YosenObject_Null->clone();

			return allocate_object<YosenString>(arg_object->instance_info());
		});
	}
}
