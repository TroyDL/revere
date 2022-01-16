# silo
#

add_executable(
    silo
    source/main.cpp
    source/mainwindow.h
    source/mainwindow.cpp
    source/mainwindow.ui
    source/utils.h
    source/utils.cpp
    source/silo.qrc
)

target_include_directories(
    silo PUBLIC 
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

target_link_directories(
    silo PUBLIC
    ${GSTREAMER_LIB_DIRS}
    ${FFMPEG_LIB_DIRS}
)

target_link_libraries(
    silo LINK_PUBLIC
    r_vss
    r_disco
    r_storage
    r_mux
    r_fakey
    r_pipeline
    r_db
    r_http
    r_utils
    ${GSTREAMER_LIBS}
    ${FFMPEG_LIBS}
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)
