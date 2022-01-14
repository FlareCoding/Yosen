#include "YosenStandardLibrary.h"
#include "Interpreter.h"
#include <iostream>

namespace yosen
{
	void YosenStandardLibrary::load_functions(Interpreter* interpreter)
	{
		interpreter->register_native_runtime_function("println", __std_println);
		interpreter->register_native_runtime_function("input", __std_input);
	}

	YosenObject* YosenStandardLibrary::__std_println(YosenObject* obj)
	{
		YosenObject* o = nullptr;
		arg_parse(obj, "o", &o);

		printf("%s\n", o->to_string().c_str());
		return YosenObject_Null->clone();
	}

	YosenObject* YosenStandardLibrary::__std_input(YosenObject* obj)
	{
		std::string input;
		std::getline(std::cin, input);

		return allocate_object<YosenString>(input);
	}
}
