#include "YosenStandardLibrary.h"

EXTERNC YOSENEXPORT void _ys_init_module()
{
	auto& env = YosenEnvironment::get();

	env.register_static_native_function("println", _ys_std_println);

	env.register_yosen_class("TestStdClass", test_std_class_builder);
}
