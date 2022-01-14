#pragma once
#include <string>
#include <fstream>
#include <filesystem>

namespace yosen::utils
{
	std::string get_current_executable_path();

	void set_terminal_color_default();
	void set_terminal_color_green();
}
