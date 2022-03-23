cmake_minimum_required(VERSION 3.14)
project(revere)

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
    revere
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

    source/utils.cpp
    source/main.cpp
)

target_include_directories(
    revere PUBLIC
    include
    ../r_motion/include
    ../r_codec/include
    ../r_vss/include
    ../r_disco/include
    ../r_storage/include
    ../r_mux/include
    ../r_fakey/include
    ../r_pipeline/include
    ../r_db/include
    ../r_http/include
    ../r_utils/include
    ${GSTREAMER_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)

find_package(OpenGL)

target_link_directories(
    revere PUBLIC
    ${GSTREAMER_LIB_DIRS}
    ${FFMPEG_LIB_DIRS}
)

target_link_libraries(
    revere
    r_motion
    r_codec
    r_disco
    r_storage
    r_mux
    r_fakey
    r_vss
    r_pipeline
    r_db
    r_http
    r_utils
    glfw
    ${OPENGL_LIBRARIES}
    tray
)
