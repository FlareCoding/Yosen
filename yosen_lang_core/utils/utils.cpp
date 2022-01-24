#include "utils.h"
#include <stdarg.h>

#ifdef _WIN32
	#include <Windows.h>
#elif defined __linux__
	#include <dlfcn.h>
#endif

namespace yosen::utils
{
	void* load_library(const std::string& name)
	{
#ifdef _WIN32
		return LoadLibraryA(name.c_str());
#elif defined __linux__
		return dlopen(name.c_str(), RTLD_LAZY);
#else
		return nullptr;
#endif
	}
	
	int free_library(void* lib)
	{
#ifdef _WIN32
		return (int)FreeLibrary(static_cast<HMODULE>(lib));
#elif defined __linux__
		dlclose(lib);
#else
		return -1;
#endif
	}
	
	void* get_proc_address(void* lib, const std::string& proc_name)
	{
#ifdef _WIN32
		return GetProcAddress(static_cast<HMODULE>(lib), proc_name.c_str());
#elif defined __linux__
		return dlsym(lib, proc_name.c_str());
#else
		return nullptr;
#endif
	}

	static std::string color_to_linux_escape_code(ConsoleColor color)
	{
		std::string code = "0";

		switch (color)
		{
		case ConsoleColor::Default: { code = "39"; break; }
		case ConsoleColor::White:	{ code = "37"; break; }
		case ConsoleColor::Red:		{ code = "31"; break; }
		case ConsoleColor::Green:	{ code = "32"; break; }
		case ConsoleColor::Blue:	{ code = "34"; break; }
		case ConsoleColor::Yellow:	{ code = "33"; break; }
		case ConsoleColor::Cyan:	{ code = "36"; break; }
		default: break;
		}

		return "\033[" + code;
	}

	static unsigned short color_to_windows_code(ConsoleColor color)
	{
		switch (color)
		{
		case ConsoleColor::Default: return 15;
		case ConsoleColor::White:	return 15;
		case ConsoleColor::Red:		return 12;
		case ConsoleColor::Green:	return 10;
		case ConsoleColor::Blue:	return 9;
		case ConsoleColor::Yellow:	return 14;
		case ConsoleColor::Cyan:	return 11;
		default: return 0;
		}
	}
	
	void log_colored(ConsoleColor color, std::string fmt, ...)
	{
#ifdef _WIN32
		// Retrieve the console handle
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		WORD old_attribs = 0;
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		// Get current original attributes
		if (GetConsoleScreenBufferInfo(hConsole, &csbi))
			old_attribs = csbi.wAttributes;

		// Set the new color attribute
		SetConsoleTextAttribute(hConsole, color_to_windows_code(color));

#else
		// Apply the color modifier to the text
		auto mod = color_to_linux_escape_code(color);
		fmt = mod + fmt + "\033[0m";
#endif

		va_list arglist;
		va_start(arglist, fmt);
		vprintf(fmt.c_str(), arglist);
		va_end(arglist);

#ifdef _WIN32
		// Restore the original console attributes
		SetConsoleTextAttribute(hConsole, old_attribs);
#endif
	}
}
