#include <filesystem>
#include <fstream>
#include "interpreter/Interpreter.h"
using namespace yosen;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("No input source file specified\n");
        return 0;
    }

    if (argc > 2)
    {
        printf("Please specify only one input source file\n");
        return 0;
    }

    std::string input_source_path = argv[1];
    if (input_source_path.empty() || !std::filesystem::is_regular_file(input_source_path))
    {
        printf("Invalid input source file specified\n");
        return 0;
    }

    std::ifstream stream(input_source_path);
    std::stringstream source_code_buffer;
    source_code_buffer << stream.rdbuf();

    // Initialize the runtime environment
    Interpreter interpreter;
    interpreter.init();

    // Run the source file
    std::string source_code = source_code_buffer.str();
    interpreter.run(source_code);

    // Free the resources and cleanup
    interpreter.shutdown();
    printf("[*] Interpreter Shutdown, total objects left allocated is {%zi}, exiting...\n", __yosen_get_total_allocated_objects());
    return 0;
}

/*
* Development Plan:

    1) [Done] Execute multiple functions that are represented as bytecode

    2) [Done] Implement a YosenTuple class and replace parameter packing in the CALL opcode parsing to use it

    3) [Done] Implement argument parsing support in the native yosen functions

    4) Make sure there are no memory leaks

    5) Build a parser that is easy to use, configure, and update/modify

    6) Make the sample test program with two functions compile from text source

    7) Implement a class system that would work as follows:
            Compiler:
                a) encounters a class definition
                b) builds a class object and adds member variables and function bytecodes to it
                c) registers a new native function in the interpreter "__yosen_instantiate_new_ClassName"
                d) and then simply call that function every time a class gets instantiated :D

    8) Implement (native/non-native) module loading and a multiple file support

    9) If/else statements

    10) While loops

    11) For loops

    12) Test recursion

    13) Implement the following primitive types:
            Integer [Done]
            String [Done]
            Char
            Boolean
            Float
            Array
            Pointer/Reference

    14) Extend the standard library to support the following classes:
            Math
            OS (system commands, filepaths, etc.)
            File (IO)
            Socket (tcp/udp)
            ...

    15) Add support for command line arguments

    16) Implement support for an interactive runtime environment where
        single line execution is allowed and doesn't require the main function to exist.
*
*/
