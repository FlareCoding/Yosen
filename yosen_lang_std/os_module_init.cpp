#include "yosen_std_os.h"

EXTERNC YOSENEXPORT void _ys_init_module()
{
	auto& env = YosenEnvironment::get();

	env.start_module_namespace("os");
	env.register_static_native_function("system",       _ys_std_os_system);
	env.register_static_native_function("chdir",        _ys_std_os_chdir);
	env.register_static_native_function("cwd",          _ys_std_os_cwd);
	env.register_static_native_function("mkdir",        _ys_std_os_mkdir);
	env.register_static_native_function("is_file",      _ys_std_os_is_file);
	env.register_static_native_function("is_dir",       _ys_std_os_is_dir);
	env.register_static_native_function("delete_file",  _ys_std_os_delete_file);
	env.register_static_native_function("delete_dir",   _ys_std_os_delete_dir);
	env.end_module_namespace();
}
