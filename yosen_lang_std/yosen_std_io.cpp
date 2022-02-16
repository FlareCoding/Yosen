#include "yosen_std_io.h"
#include <iostream>

YosenObject* _ys_std_io_print(YosenObject* args)
{
	YosenObject* obj = nullptr;
	arg_parse(args, "o", &obj);

	if (!obj)
	{
		// Free the parameter pack
		free_object(args);

		auto ex_reason = "print() expects 1 argument";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
	}

	printf("%s", obj->to_string().c_str());
	return YosenObject_Null->clone();
}

YosenObject* _ys_std_io_println(YosenObject* args)
{
	YosenObject* obj = nullptr;
	arg_parse(args, "o", &obj);

	if (!obj)
	{
		// Free the parameter pack
		free_object(args);

		auto ex_reason = "println() expects 1 argument";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
	}

	printf("%s\n", obj->to_string().c_str());
	return YosenObject_Null->clone();
}

YosenObject* _ys_std_io_input(YosenObject* args)
{
	if (static_cast<YosenTuple*>(args)->items.size())
	{
		YosenObject* obj = nullptr;
		arg_parse(args, "o", &obj);

		printf("%s", obj->to_string().c_str());
	}
	else
		arg_parse(args, "");

	std::string input;
	std::getline(std::cin, input);

	return allocate_object<YosenString>(input);
}
