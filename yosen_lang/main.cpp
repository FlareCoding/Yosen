#include <filesystem>
#include <fstream>
#include <sstream>

#include "interpreter/YosenInterpreter.h"
using namespace yosen;

int main(int argc, char** argv)
{
    std::string source_code = "";
    std::vector<std::string> cmd_arguments;

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
            cmd_arguments.push_back(argv[i]);
    }

    if (argc > 1)
    {
        std::string input_source_path = argv[1];
        if (input_source_path.empty() || !std::filesystem::is_regular_file(input_source_path))
        {
            printf("Invalid input source file specified\n");
            return 0;
        }

        std::ifstream stream(input_source_path);
        std::stringstream source_code_buffer;
        source_code_buffer << stream.rdbuf();

        source_code = source_code_buffer.str();
    }

	auto interpreter = std::make_unique<YosenInterpreter>();
	interpreter->init();

    if (argc == 1)
        interpreter->run_interactive_shell();
    else
        interpreter->run_source(source_code, cmd_arguments);

	interpreter->shutdown();
	return 0;
}
