#pragma once

#include "Ignis.h"

// Platform-specific headers for executable path detection
#if defined(__APPLE__)
	#include <mach-o/dyld.h>  // For _NSGetExecutablePath
#elif defined(_WIN32)
	#include <windows.h>       // For GetModuleFileName
#else
	#include <unistd.h>        // For readlink
#endif

class Sandbox : public ignis::Application
{
public:
	Sandbox();
	~Sandbox() = default;

private:

};