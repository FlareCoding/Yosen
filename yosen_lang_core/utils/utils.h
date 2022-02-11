#pragma once
#include <YosenCore.h>
#include <string>
#include <filesystem>
#include <functional>

namespace yosen::utils
{
	enum class ConsoleColor
	{
		Default,
		White,
		Red,
		Green,
		Blue,
		Yellow,
		Cyan
	};

	YOSENAPI void*			load_library(const std::string& name);
	YOSENAPI int			free_library(void* lib);
	YOSENAPI void*			get_proc_address(void* lib, const std::string& proc_name);

	YOSENAPI std::string	get_current_executable_path();
	YOSENAPI std::string 	get_current_path();
	YOSENAPI std::string	get_installation_path();

	YOSENAPI void			log_colored(ConsoleColor color, std::string fmt, ...);

	YOSENAPI void			set_keyboard_interrupt_handler(std::function<void()> handler);
}
