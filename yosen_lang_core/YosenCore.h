#pragma once
#ifdef _WIN32
	#ifdef YOSEN_BUILD
		#define YOSENAPI __declspec(dllexport)
	#else
		#define YOSENAPI __declspec(dllimport)
	#endif

	#define YOSENEXPORT __declspec(dllexport)
	#define YOSENIMPORT __declspec(dllimport)
#else
	#define YOSENAPI
	#define YOSENEXPORT
	#define YOSENIMPORT
#endif

#define EXTERNC extern "C"

#ifdef DEBUG
	#define YOSEN_INTERPRETER_DEBUG_MODE 1
#endif