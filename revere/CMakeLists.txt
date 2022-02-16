# revere
#

add_executable(
    revere
    source/main.cpp
    source/mainwindow.h
    source/mainwindow.cpp
    source/mainwindow.ui
    source/utils.h
    source/utils.cpp
    source/revere.qrc
    source/rtspcredentials.ui
    source/rtspcredentials.h
    source/rtspcredentials.cpp
    source/friendlyname.ui
    source/friendlyname.h
    source/friendlyname.cpp
    source/retention.ui
    source/retention.h
    source/retention.cpp
    source/neworexisting.ui
    source/neworexisting.h
    source/neworexisting.cpp
    source/pleasewait.ui
    source/pleasewait.h
    source/pleasewait.cpp
    source/newfilename.ui
    source/newfilename.h
    source/newfilename.cpp
    source/ws.h
    source/ws.cpp
)

target_include_directories(
    revere PUBLIC 
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

target_link_directories(
    revere PUBLIC
    ${GSTREAMER_LIB_DIRS}
    ${FFMPEG_LIB_DIRS}
)

target_link_libraries(
    revere LINK_PUBLIC
    r_codec
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
    Qt5::UiTools
    Qt5::Widgets
    Qt5::Multimedia
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)
