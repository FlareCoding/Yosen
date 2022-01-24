#pragma once
#include <YosenCore.h>
#include <string>
#include <filesystem>

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

	YOSENAPI void*	load_library(const std::string& name);
	YOSENAPI int	free_library(void* lib);
	YOSENAPI void*	get_proc_address(void* lib, const std::string& proc_name);

	YOSENAPI void	log_colored(ConsoleColor color, std::string fmt, ...);
}
