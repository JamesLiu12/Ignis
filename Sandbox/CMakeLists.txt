file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox
	PRIVATE Ignis::Ignis
)