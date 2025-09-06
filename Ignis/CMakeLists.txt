file(GLOB_RECURSE IGNIS_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(Ignis STATIC ${IGNIS_SOURCES})
add_library(Ignis::Ignis ALIAS Ignis)

target_include_directories(Ignis PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Ignis PUBLIC spdlog::spdlog)

target_precompile_headers(Ignis
    PUBLIC
        $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>
)