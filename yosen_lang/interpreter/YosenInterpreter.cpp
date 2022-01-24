#include "YosenInterpreter.h"

namespace yosen
{
	YosenEnvironment s_dummy_env;

	YosenInterpreter::YosenInterpreter()
		: m_env(s_dummy_env)
	{
	}
	
	void YosenInterpreter::init()
	{
		// Initialize the current environment
		YosenEnvironment::init();

		m_env = YosenEnvironment::get();

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
		utils::log_colored(
			utils::ConsoleColor::Green,
			"[*] Yosen Interpreter Initialized\n"
		);
#endif
	}
	
	void YosenInterpreter::shutdown()
	{
		// Shutdown the environment
		m_env.shutdown();

#if (YOSEN_INTERPRETER_DEBUG_MODE == 1)
		utils::log_colored(
			utils::ConsoleColor::Green,
			"[*] Interpreter Shutdown, total objects left allocated is {%zi}, exiting...\n",
			__yosen_get_total_allocated_objects()
		);
#endif
	}
	
	void YosenInterpreter::run_interactive_shell()
	{
		YosenObject* obj = m_env.construct_class_instance("TestStdClass");
		auto native_result = obj->call_member_native_function("test_fn", YosenObject_Null);

		auto fn = m_env.get_static_native_function("println");
		if (fn)
		{
			std::vector<YosenObject*> arg_vec = { obj->clone() };
			auto args = allocate_object<YosenTuple>(arg_vec);
			auto result = fn(args);

			free_object(result);
			free_object(args);
		}

		free_object(obj);
		free_object(native_result);
	}
}
