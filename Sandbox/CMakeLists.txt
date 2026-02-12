file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_include_directories(Sandbox PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Sandbox
	PRIVATE Ignis::Ignis
)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(DEST_DIR   "${CMAKE_CURRENT_BINARY_DIR}/assets")

add_custom_command(
    OUTPUT "${DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SOURCE_DIR}" "${DEST_DIR}"
    DEPENDS "${SOURCE_DIR}"
    COMMENT "Copying asset directory"
)

add_custom_target(copy_assets ALL DEPENDS "${DEST_DIR}")