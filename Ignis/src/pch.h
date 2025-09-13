#pragma once

// OpenGL deprecation suppression - must be first
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
#endif

#include <iostream>
#include <memory>
#include <algorithm>
#include <functional>
#include <mutex>
#include <utility>
#include <atomic>
#include <thread>
#include <chrono>
#include <filesystem>
#include <typeindex>

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <map>

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

// spdlog headers
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Ignis/Core/Log.h"