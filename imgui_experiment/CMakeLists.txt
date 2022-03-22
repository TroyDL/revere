cmake_minimum_required(VERSION 3.14)
project(imgui_experiment)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG 3.3-stable
)

FetchContent_MakeAvailable(glfw)

FetchContent_Declare(tray
  GIT_REPOSITORY https://github.com/dicroce/traypp
  GIT_TAG master
)

FetchContent_MakeAvailable(tray)

add_executable(
    imgui_experiment
    include/imconfig.h
    include/imgui_internal.h
    include/imstb_rectpack.h
    include/imstb_textedit.h
    include/imstb_truetype.h
    include/imgui.h
    source/imgui.cpp
    source/imgui_demo.cpp
    source/imgui_draw.cpp
    source/imgui_tables.cpp
    source/imgui_widgets.cpp
    source/imgui_impl_glfw.cpp
    source/imgui_impl_opengl3.cpp
    source/main.cpp
)

target_include_directories(
    imgui_experiment PUBLIC
    include
)

find_package(OpenGL)

target_link_libraries(
    imgui_experiment
    glfw
    ${OPENGL_LIBRARIES}
    tray
)
