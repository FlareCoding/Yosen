#pragma once
#include <YosenEnvironment.h>
using namespace yosen;

// Executes a system command and returns its output as a string
YosenObject* _ys_std_os_system(YosenObject* args);

// Changes current working directory
YosenObject* _ys_std_os_chdir(YosenObject* args);

// Returns the current working directory path as a string
YosenObject* _ys_std_os_cwd(YosenObject* args);

// Creates a new directory if it doesn't exist and returns
// true, if the directory already exists, return false.
YosenObject* _ys_std_os_mkdir(YosenObject* args);

// Returns true if the specified path is a valid file
YosenObject* _ys_std_os_is_file(YosenObject* args);

// Returns true if the specified path is a folder/directory
YosenObject* _ys_std_os_is_dir(YosenObject* args);

// Returns 0 if successfully deleted a file,
// otherwise a system error code is returned.
YosenObject* _ys_std_os_delete_file(YosenObject* args);

// Returns 0 if successfully deleted a directory,
// otherwise a system error code is returned.
YosenObject* _ys_std_os_delete_dir(YosenObject* args);
