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
        nlohmannjson
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

# Platform-specific file dialog implementation
if(APPLE)
    target_sources(Ignis PRIVATE Ignis/Platform/macOS/macOSFileDialog.mm)
    # Disable PCH for Objective-C++ file (incompatible with C++ PCH)
    set_source_files_properties(Ignis/Platform/macOS/macOSFileDialog.mm PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    # Link Cocoa framework for macOS
    target_link_libraries(Ignis PRIVATE "-framework Cocoa")
elseif(WIN32)
    target_sources(Ignis PRIVATE Ignis/Platform/Windows/WindowsFileDialog.cpp)
    # Link Windows common dialogs library
    target_link_libraries(Ignis PRIVATE comdlg32)
endif()

target_include_directories(Ignis PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(Ignis
    PUBLIC
        $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>
)