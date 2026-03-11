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

# Add runtime subdirectory (if it exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Runtime")
	add_subdirectory(Runtime)
endif()