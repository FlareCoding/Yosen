#include "yosen_std_io.h"

EXTERNC YOSENEXPORT void _ys_init_module()
{
	auto& env = YosenEnvironment::get();

	env.start_module_namespace("io");
	env.register_static_native_function("print",	_ys_std_io_print);
	env.register_static_native_function("println",	_ys_std_io_println);
	env.register_static_native_function("input",	_ys_std_io_input);
	env.end_module_namespace();
}
