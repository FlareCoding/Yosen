#include "utils.h"

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

namespace yosen::utils
{
	std::string yosen::utils::get_current_executable_path()
	{
#if defined(PLATFORM_POSIX) || defined(__linux__)
        std::string sp;
        std::ifstream("/proc/self/comm") >> sp;
        return sp;
#elif defined(_WIN32)
        char buf[MAX_PATH];
        GetModuleFileNameA(nullptr, buf, MAX_PATH);
        return buf;
#else
        static_assert(false, "Platform not supported");
#endif
	}

    void set_terminal_color_default()
    {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, 15);
#endif // _WIN32
    }

    void set_terminal_color_green()
    {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, 10);
#endif // _WIN32
    }
}
