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
			throw std::exception("Yosen environment already initialized!");
			return;
		}

		// Initialize default objects
		YosenObject_Null = allocate_object<YosenObject>();
		YosenObject_Null->m_string_repr = "null";

		// Initialize standard library
		s_env_instance->initialize_standard_library();

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
		utils::log_colored(
			utils::ConsoleColor::Green,
			"[*] Yosen Environment Initialized\n"
		);
#endif
	}

	void YosenEnvironment::shutdown()
	{
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

	void YosenEnvironment::initialize_standard_library()
	{
		load_yosen_module("yosen_std_math");
	}
}
