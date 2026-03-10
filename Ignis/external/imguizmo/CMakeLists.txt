add_library(imguizmo SHARED include/ImGuizmo.cpp)
target_include_directories(imguizmo PUBLIC include)
target_link_libraries(imguizmo PUBLIC imgui)