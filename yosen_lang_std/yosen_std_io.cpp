#include "yosen_std_io.h"
#include <iostream>

YosenObject* _ys_std_io_print(YosenObject* args)
{
	YosenObject* obj;
	arg_parse(args, "o", &obj);

	printf("%s", obj->to_string().c_str());
	return YosenObject_Null->clone();
}

YosenObject* _ys_std_io_println(YosenObject* args)
{
	YosenObject* obj;
	arg_parse(args, "o", &obj);

	printf("%s\n", obj->to_string().c_str());
	return YosenObject_Null->clone();
}

YosenObject* _ys_std_io_input(YosenObject* args)
{
	if (static_cast<YosenTuple*>(args)->items.size())
	{
		YosenObject* obj;
		arg_parse(args, "o", &obj);

		printf("%s", obj->to_string().c_str());
	}

	std::string input;
	std::getline(std::cin, input);

	return allocate_object<YosenString>(input);
}
