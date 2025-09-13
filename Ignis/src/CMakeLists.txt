file(GLOB_RECURSE IGNIS_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(Ignis STATIC ${IGNIS_SOURCES})
add_library(Ignis::Ignis ALIAS Ignis)

target_link_libraries(Ignis PUBLIC
     glfw
     spdlog::spdlog
     imgui
)

target_include_directories(Ignis PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(Ignis
    PUBLIC
        $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>
)