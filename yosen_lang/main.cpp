#include <filesystem>
#include <fstream>
#include <sstream>

#include "interpreter/YosenInterpreter.h"
using namespace yosen;

int main(int argc, char** argv)
{
    std::string source_code = "";

    if (argc > 2)
    {
        printf("Please specify only one input source file\n");
        return 0;
    }

    if (argc == 2)
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

    if (source_code.empty())
        interpreter->run_interactive_shell();
    else
        interpreter->run_source(source_code);

	interpreter->shutdown();
	return 0;
}
