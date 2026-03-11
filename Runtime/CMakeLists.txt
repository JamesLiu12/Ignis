project(IgnisRuntime)

# Collect all runtime source files
file(GLOB_RECURSE RUNTIME_SOURCES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Runtime executable - use a different target name to avoid conflict with script module
add_executable(IgnisRuntime
	${RUNTIME_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/../Ignis/src/Ignis/Core/EntryPoint.cpp
)

# Link against engine
target_link_libraries(IgnisRuntime PRIVATE Ignis::Ignis)

# Include directories
target_include_directories(IgnisRuntime PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Get the root project name (IgnisExampleProject) for the output executable name
get_directory_property(ROOT_PROJECT_NAME DIRECTORY ${CMAKE_SOURCE_DIR} DEFINITION CMAKE_PROJECT_NAME)

# Set output directory to match Editor (same as CMAKE_RUNTIME_OUTPUT_DIRECTORY from root)
set_target_properties(IgnisRuntime PROPERTIES
	# Use root project name for executable (e.g., IgnisExampleProject.exe or IgnisExampleProject)
	OUTPUT_NAME "${ROOT_PROJECT_NAME}"
	
	# Use same output directory as Editor
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

# Windows: Copy DLL dependencies automatically
if(WIN32)
	# This tells CMake to copy all dependent DLLs to the output directory
	add_custom_command(TARGET IgnisRuntime POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_RUNTIME_DLLS:IgnisRuntime>
			$<TARGET_FILE_DIR:IgnisRuntime>
		COMMAND_EXPAND_LISTS
	)
endif()

# UTF-8 support for MSVC
if(MSVC)
	target_compile_options(IgnisRuntime PRIVATE /utf-8)
endif()
