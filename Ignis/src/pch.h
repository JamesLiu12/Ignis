#pragma once

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

// ImGui headers
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// GLFW headers
#include <GLFW/glfw3.h>