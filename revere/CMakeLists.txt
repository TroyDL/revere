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
    source/motiondetection.ui
    source/motiondetection.h
    source/motiondetection.cpp
    source/ws.h
    source/ws.cpp
)

if(MSVC)
    target_link_options(revere PUBLIC "/STACK:10000000")
endif()

target_include_directories(
    revere PUBLIC
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

target_link_directories(
    revere PUBLIC
    ${GSTREAMER_LIB_DIRS}
    ${FFMPEG_LIB_DIRS}
)

set(REVERE_LIBS
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
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(
        revere
        ${REVERE_LIBS}
        ${GSTREAMER_LIBS}
        ${FFMPEG_LIBS}
        Qt6::Widgets
        Qt6::Multimedia
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(
        revere
        ${REVERE_LIBS}
        ${GSTREAMER_LIBS}
        ${FFMPEG_LIBS}
        Qt5::Widgets
        Qt5::Multimedia
        ${CMAKE_THREAD_LIBS_INIT}
        ${CMAKE_DL_LIBS}
    )
endif()
