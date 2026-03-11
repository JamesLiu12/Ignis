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

# Define _DEBUG for Debug builds
target_compile_definitions(IgnisRuntime PRIVATE
	$<$<CONFIG:Debug>:_DEBUG>
)

# Include directories
target_include_directories(IgnisRuntime PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set output directory to match Editor (same as CMAKE_RUNTIME_OUTPUT_DIRECTORY from root)
set_target_properties(IgnisRuntime PROPERTIES
	# Keep default name "IgnisRuntime" - export will rename to project name
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
