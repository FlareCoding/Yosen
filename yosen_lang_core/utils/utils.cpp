#include "utils.h"
#include <stdarg.h>
#include <fstream>

#ifdef _WIN32
	#include <Windows.h>
#else
	#include <dlfcn.h>
	#include <signal.h>
	#include <stdlib.h>

	#ifdef __APPLE__
		#include <mach-o/dyld.h>
		#include <sys/syslimits.h>
	#elif defined(__linux__)
		#include <unistd.h>
		#include <limits.h>
		#include <cstring>
	#endif
#endif

namespace yosen::utils
{
	static std::function<void()> s_keyboard_interrupt_handler_ref = nullptr;

	void* load_library(const std::string& name)
	{
		auto mod_dir = get_installation_path();

#ifdef _WIN32
		// First attempt to look in the current directory
		auto local_mod = LoadLibraryA(name.c_str());
		if (local_mod)
			return local_mod;

		return LoadLibraryA((mod_dir + name).c_str());
#elif defined(__APPLE__)
		// First attempt to look in the current directory
		auto local_result = dlopen(("lib" + name + ".dylib").c_str(), RTLD_LAZY);
		if (local_result)
			return local_result;
		
		auto result = dlopen((mod_dir + "lib" + name + ".dylib").c_str(), RTLD_LAZY);
		if (!result)
			printf("dlerror(): %s\n", dlerror());

		return result;
#else
		// First attempt to look in the current directory
		auto local_result = dlopen(("./lib" + name + ".so").c_str(), RTLD_LAZY);
		if (local_result)
			return local_result;

		auto result = dlopen((mod_dir + "lib" + name + ".so").c_str(), RTLD_LAZY);
		if (!result)
			printf("dlerror(): %s\n", dlerror());

		return result;
#endif
	}
	
	int free_library(void* lib)
	{
#ifdef _WIN32
		return (int)FreeLibrary(static_cast<HMODULE>(lib));
#else
		return dlclose(lib);
#endif
	}
	
	void* get_proc_address(void* lib, const std::string& proc_name)
	{
#ifdef _WIN32
		return GetProcAddress(static_cast<HMODULE>(lib), proc_name.c_str());
#else
		return dlsym(lib, proc_name.c_str());
#endif
	}

	std::string get_current_executable_path()
	{
#ifdef _WIN32
		char buf[MAX_PATH];
		GetModuleFileNameA(nullptr, buf, MAX_PATH);
		return buf;
#elif defined(__APPLE__)
		char sym_path[PATH_MAX + 1];
		char real_path[PATH_MAX + 1];
		uint32_t size = sizeof(sym_path);

		// Get symbolic path
		_NSGetExecutablePath(sym_path, &size);

		// Get real path
		realpath(sym_path, real_path);

		return real_path;
#else
		char buf[PATH_MAX];
		memset(buf, 0, sizeof(buf));

		auto _ = readlink("/proc/self/exe", buf, PATH_MAX);
		return buf;
#endif
	}

	std::string get_current_path()
	{
		return std::filesystem::current_path().string();
	}

	std::string get_installation_path()
	{
#ifdef _WIN32
			return "C:\\yosen_lang\\";
#else
			return "/usr/local/bin/yosen_lang/";
#endif
	}

	static std::string color_to_ansii_code(ConsoleColor color)
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

		return code;
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
	#ifdef __APPLE__
		std::string prefix = "\x1b[";
	#else
		std::string prefix = "\033[";
	#endif
	
		// Apply the color modifier to the text
		auto mod = prefix + color_to_ansii_code(color) + "m";
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
	
	void set_keyboard_interrupt_handler(std::function<void()> handler)
	{
		s_keyboard_interrupt_handler_ref = handler;

#ifdef _WIN32
		SetConsoleCtrlHandler([](DWORD dwCtrlType) {
			s_keyboard_interrupt_handler_ref();
			return FALSE;
		}, TRUE);
#else
		struct sigaction sigIntHandler;

		sigIntHandler.sa_handler = [](int) { s_keyboard_interrupt_handler_ref(); };
		sigemptyset(&sigIntHandler.sa_mask);
		sigIntHandler.sa_flags = 0;

		sigaction(SIGINT, &sigIntHandler, NULL);
#endif
	}
}
