#pragma once
#include <YosenEnvironment.h>

namespace yosen
{
	class YosenInterpreter
	{
	public:
		YosenInterpreter();

		void init();
		void shutdown();

		void run_interactive_shell();

	private:
		YosenEnvironment& m_env;
	};
}
