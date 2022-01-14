#pragma once
#include "../native_objects/YosenObject.h"
#include "../native_objects/YosenInteger.h"
#include "../native_objects/YosenString.h"

namespace yosen
{
	class Interpreter;

	class YosenStandardLibrary
	{
	public:
		static void load_functions(Interpreter* interpreter);

	private:
		static YosenObject* __std_println(YosenObject* obj);
		static YosenObject* __std_input(YosenObject* obj);
	};
}
