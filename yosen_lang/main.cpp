#include "interpreter/YosenInterpreter.h"
using namespace yosen;

int main()
{
	auto interpreter = std::make_unique<YosenInterpreter>();
	interpreter->init();

	interpreter->run_interactive_shell();

	interpreter->shutdown();
	return 0;
}
