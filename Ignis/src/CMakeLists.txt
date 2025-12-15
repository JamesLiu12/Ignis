file(GLOB_RECURSE IGNIS_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(Ignis STATIC ${IGNIS_SOURCES})
add_library(Ignis::Ignis ALIAS Ignis)

target_link_libraries(Ignis 
    PUBLIC
        spdlog::spdlog
        imgui
        glfw
        glad
        glm
        BulletDynamics 
        BulletCollision 
        LinearMath
        stb
        entt
        assimp
        stduuid
)

target_include_directories(Ignis PUBLIC
    ${BULLET_PHYSICS_SOURCE_DIR}/src
)

# Platform-specific OpenGL linking
if(APPLE)
    # Link OpenGL framework on macOS
    target_link_libraries(Ignis PRIVATE "-framework OpenGL")
elseif(WIN32)
    # Link OpenGL library on Windows
    target_link_libraries(Ignis PRIVATE opengl32)
elseif(UNIX)
    # Link OpenGL library on Linux
    find_package(OpenGL REQUIRED)
    target_link_libraries(Ignis PRIVATE OpenGL::GL)
endif()

target_include_directories(Ignis PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(Ignis
    PUBLIC
        $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>
)