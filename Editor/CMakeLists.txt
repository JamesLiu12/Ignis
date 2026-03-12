file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(Editor 
    ${EDITOR_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Ignis/src/Ignis/Core/EntryPoint.cpp
)

# Define _DEBUG for Debug builds
target_compile_definitions(Editor PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
)

if(MSVC)
    target_compile_options(Editor PRIVATE /utf-8)
endif()

target_include_directories(Editor PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Editor
	PRIVATE Ignis::Ignis
)

# macOS: Use relative RPATH so Editor is portable
if(APPLE)
	set_target_properties(Editor PROPERTIES
		BUILD_WITH_INSTALL_RPATH TRUE
		INSTALL_RPATH "@executable_path"
	)
endif()

set(SOURCE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(DEST_DIR    "$<TARGET_FILE_DIR:Editor>/resources")
set(STAMP_FILE  "${CMAKE_CURRENT_BINARY_DIR}/copy_resources.stamp")

file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*")

add_custom_command(
    OUTPUT  "${STAMP_FILE}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SOURCE_DIR}" "${DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E touch "${STAMP_FILE}"
    DEPENDS ${RESOURCE_FILES}
    COMMENT "Copying resources directory"
)

add_custom_target(copy_resources ALL DEPENDS "${STAMP_FILE}")

target_precompile_headers(Editor PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>
)