cmake_minimum_required(VERSION 3.20)

project(Ignis
  VERSION 0.1.0
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(Ignis)
add_subdirectory(Editor)

# Runtime is optional - only build if explicitly requested
option(BUILD_RUNTIME "Build runtime executable" OFF)
if(BUILD_RUNTIME)
	add_subdirectory(Runtime)
endif()