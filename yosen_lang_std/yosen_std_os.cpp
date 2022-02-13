#include "yosen_std_os.h"
#include <string>
#include <array>
#include <filesystem>
#include <stdio.h>

YosenObject* _ys_std_os_system(YosenObject* args)
{
    char* cmd = nullptr;
	arg_parse(args, "s", &cmd);

	if (!cmd)
	{
		// Free the parameter pack
		free_object(args);

		auto ex_reason = "os::system() expected 1 string argument: cmd";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
	}

    std::array<char, 4096> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return allocate_object<YosenString>(result);
}

YosenObject* _ys_std_os_chdir(YosenObject* args)
{
    char* path = nullptr;
	arg_parse(args, "s", &path);

	if (!path)
	{
		// Free the parameter pack
		free_object(args);

		auto ex_reason = "os::chdir() expected 1 string argument: path";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
	}

    if (std::filesystem::is_directory(path))
    {
        std::filesystem::current_path(path);
        return allocate_object<YosenBoolean>(true);
    }

    return allocate_object<YosenBoolean>(false);
}

YosenObject* _ys_std_os_cwd(YosenObject* args)
{
    if (static_cast<YosenTuple*>(args)->items.size())
    {
        // Free the parameter pack
		free_object(args);

		auto ex_reason = "os::cwd() expected 0 arguments";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
    }

    return allocate_object<YosenString>(std::filesystem::current_path().string());
}

YosenObject* _ys_std_os_mkdir(YosenObject* args)
{
    char* path = nullptr;
	arg_parse(args, "s", &path);

	if (!path)
	{
		// Free the parameter pack
		free_object(args);

		auto ex_reason = "os::mkdir() expected 1 string argument: path";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
	}

    if (std::filesystem::exists(path))
        return allocate_object<YosenBoolean>(false);

    bool result = std::filesystem::create_directory(path);
    return allocate_object<YosenBoolean>(result);
}

YosenObject* _ys_std_os_is_file(YosenObject* args)
{
    char* path = nullptr;
	arg_parse(args, "s", &path);

	if (!path)
	{
		// Free the parameter pack
		free_object(args);

		auto ex_reason = "os::is_file() expected 1 string argument: path";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
	}

    bool result = !std::filesystem::is_directory(path);
    return allocate_object<YosenBoolean>(result);
}

YosenObject* _ys_std_os_is_dir(YosenObject* args)
{
    char* path = nullptr;
	arg_parse(args, "s", &path);

	if (!path)
	{
		// Free the parameter pack
		free_object(args);

		auto ex_reason = "os::is_dir() expected 1 string argument: path";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
	}

    bool result = std::filesystem::is_directory(path);
    return allocate_object<YosenBoolean>(result);
}

YosenObject* _ys_std_os_delete_file(YosenObject* args)
{
    char* path = nullptr;
	arg_parse(args, "s", &path);

	if (!path)
	{
		// Free the parameter pack
		free_object(args);

		auto ex_reason = "os::delete_file() expected 1 string argument: path";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
	}

    bool result = std::filesystem::remove(path);
    return allocate_object<YosenBoolean>(result);
}

YosenObject* _ys_std_os_delete_dir(YosenObject* args)
{
    char* path = nullptr;
	arg_parse(args, "s", &path);

	if (!path)
	{
		// Free the parameter pack
		free_object(args);

		auto ex_reason = "os::delete_file() expected 1 string argument: path";
		YosenEnvironment::get().throw_exception(RuntimeException(ex_reason));
        return nullptr;
	}

    auto deleted_count = std::filesystem::remove_all(path);
    
    if (deleted_count > 0)
        return allocate_object<YosenBoolean>(true);
    else
        return allocate_object<YosenBoolean>(false);
}

