#include "yosen_std_random.h"
#include <random>
#include <time.h>

EXTERNC YOSENEXPORT void _ys_init_module()
{
	srand(time(NULL));

	auto& env = YosenEnvironment::get();

	env.start_module_namespace("random");
	env.register_static_native_function("gen_int",          _ys_std_random_gen_int);
	env.register_static_native_function("gen_int_range",    _ys_std_random_gen_int_range);
	env.register_static_native_function("gen_string",       _ys_std_random_gen_string);
	env.register_static_native_function("gen_hex_string",   _ys_std_random_gen_hex_string);
	env.register_static_native_function("gen_uuid",         _ys_std_random_gen_uuid);
	env.end_module_namespace();
}
