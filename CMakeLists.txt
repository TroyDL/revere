cmake_minimum_required(VERSION 2.8.12)
project(revere)
include(TestBigEndian)

# Global Config

if(IS_BIG_ENDIAN)
    add_compile_definitions(BYTE_ORDER=BIG_ENDIAN)
else()
    add_compile_definitions(BYTE_ORDER=LITTLE_ENDIAN)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Debug)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_compile_definitions(IS_WINDOWS)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_compile_definitions(IS_LINUX)
endif()

find_package(Threads)

find_package(LibXml2)

# Global Qt Config

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Qml Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}Multimedia REQUIRED)

# 3rd Party Deps

# GStreamer
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(
    GSTREAMER_INCLUDE_DIRS
    deps/windows/gstreamer/1.0/msvc_x86_64/include/gstreamer-1.0
    deps/windows/gstreamer/1.0/msvc_x86_64/include/glib-2.0
    deps/windows/gstreamer/1.0/msvc_x86_64/lib/glib-2.0/include
)

set(
    GSTREAMER_LIB_DIRS
    deps/windows/gstreamer/1.0/msvc_x86_64/lib
)

set(
    GSTREAMER_LIBS
    gstreamer-1.0
    gstapp-1.0
    gstsdp-1.0
    gstrtspserver-1.0
    gstcodecparsers-1.0
    gobject-2.0
    gmodule-2.0
    xml2
    gthread-2.0
    glib-2.0
)

endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

find_package(PkgConfig)
pkg_check_modules(GSTREAMER_CORE REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST_SDP REQUIRED gstreamer-sdp-1.0)
pkg_check_modules(GST_RTSP_SERVER REQUIRED gstreamer-rtsp-server-1.0)
pkg_check_modules(GST_CODECPARSERS REQUIRED gstreamer-codecparsers-1.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(GMODULE REQUIRED gmodule-2.0)
pkg_check_modules(GTHREAD REQUIRED gthread-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)

set(
    GSTREAMER_INCLUDE_DIRS
    ${GSTREAMER_CORE_INCLUDE_DIRS}
    ${GST_APP_INCLUDE_DIRS}
    ${GST_SDP_INCLUDE_DIRS}
    ${GST_RTSP_SERVER_INCLUDE_DIRS}
    ${GST_CODECPARSERS_INCLUDE_DIRS}
    ${GOBJECT_INCLUDE_DIRS}
    ${GMODULE_INCLUDE_DIRS}
    ${GTHREAD_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)

set(
    GSTREAMER_LIB_DIRS
    ${GSTREAMER_CORE_LIBRARY_DIRS}
    ${GST_APP_LIBRARY_DIRS}
    ${GST_SDP_LIBRARY_DIRS}
    ${GST_RTSP_SERVER_LIBRARY_DIRS}
    ${GST_CODECPARSERS_LIBRARY_DIRS}
    ${GOBJECT_LIBRARY_DIRS}
    ${GMODULE_LIBRARY_DIRS}
    ${GTHREAD_LIBRARY_DIRS}
    ${GLIB_LIBRARY_DIRS}
)

set(
    GSTREAMER_LIBS
    ${GSTREAMER_CORE_LIBRARIES}
    ${GST_APP_LIBRARIES}
    ${GST_SDP_LIBRARIES}
    ${GST_RTSP_SERVER_LIBRARIES}
    ${GST_CODECPARSERS_LIBRARIES}
    ${GOBJECT_LIBRARY_LIBRARIES}
    ${GMODULE_LIBRARY_LIBRARIES}
    ${GTHREAD_LIBRARY_LIBRARIES}
    ${GLIB_LIBRARY_LIBRARIES}
)

endif()

#FFmpeg

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(
    FFMPEG_INCLUDE_DIRS
    deps/windows/ffmpeg/include
)

set(
    FFMPEG_LIB_DIRS
    deps/windows/ffmpeg/lib
)

set(
    FFMPEG_LIBS
    swscale
    avdevice
    avformat
    avcodec
    avutil
)

endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

pkg_search_module(SWSCALE REQUIRED libswscale) 
pkg_search_module(AVDEVICE REQUIRED libavdevice) 
pkg_search_module(AVFORMAT REQUIRED libavformat) 
pkg_search_module(AVCODEC REQUIRED libavcodec) 
pkg_search_module(AVUTIL REQUIRED libavutil) 

set(
    FFMPEG_INCLUDE_DIRS
    ${SWSCALE_INCLUDE_DIRS}
    ${AVDEVICE_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVCODEC_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
)

set(
    FFMPEG_LIB_DIRS
    ${SWSCALE_LIB_DIRS}
    ${AVDEVICE_LIB_DIRS}
    ${AVFORMAT_LIB_DIRS}
    ${AVCODEC_LIB_DIRS}
    ${AVUTIL_LIB_DIRS}
)

set(
    FFMPEG_LIBS
    ${SWSCALE_LIBRARIES}
    ${AVDEVICE_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${AVUTIL_LIBRARIES}
)

endif()

# Libraries
#

# r_utils
#
add_library(
    r_utils
    r_utils/include/r_utils/r_actor.h
    r_utils/include/r_utils/r_macro.h
    r_utils/include/r_utils/r_args.h
    r_utils/source/r_args.cpp
    r_utils/include/r_utils/r_file_lock.h
    r_utils/source/r_file_lock.cpp
    r_utils/include/r_utils/r_logger.h
    r_utils/source/r_logger.cpp
    r_utils/include/r_utils/r_string_utils.h
    r_utils/source/r_string_utils.cpp
    r_utils/include/r_utils/r_file.h
    r_utils/source/r_file.cpp
    r_utils/include/r_utils/r_exception.h
    r_utils/source/r_exception.cpp
    r_utils/include/r_utils/r_stack_trace.h
    r_utils/source/r_stack_trace.cpp
    r_utils/include/r_utils/r_socket_address.h
    r_utils/source/r_socket_address.cpp
    r_utils/include/r_utils/interfaces/r_pollable.h
    r_utils/include/r_utils/interfaces/r_socket_io.h
    r_utils/include/r_utils/interfaces/r_stream_io.h
    r_utils/include/r_utils/r_socket.h
    r_utils/source/r_socket.cpp
    r_utils/include/r_utils/r_md5.h
    r_utils/source/r_md5.cpp
    r_utils/include/r_utils/r_time_utils.h
    r_utils/source/r_time_utils.cpp
    r_utils/include/r_utils/r_udp_sender.h
    r_utils/source/r_udp_sender.cpp
    r_utils/include/r_utils/r_udp_receiver.h
    r_utils/source/r_udp_receiver.cpp
    r_utils/include/r_utils/r_byte_ptr.h
    r_utils/source/r_byte_ptr.cpp
    r_utils/include/r_utils/r_algorithms.h
    r_utils/source/r_algorithms.cpp
    r_utils/include/r_utils/r_uuid.h
    r_utils/source/r_uuid.cpp
    r_utils/include/r_utils/r_nullable.h
    r_utils/source/r_nullable.cpp
    r_utils/include/r_utils/r_blob_tree.h
    r_utils/source/r_blob_tree.cpp
    r_utils/include/r_utils/r_std_utils.h
    r_utils/source/r_std_utils.cpp
    r_utils/include/r_utils/r_memory_map.h
    r_utils/source/r_memory_map.cpp
    r_utils/include/r_utils/r_timer.h
    r_utils/source/r_timer.cpp
    r_utils/include/r_utils/r_sha1.h
    r_utils/source/r_sha1.cpp
    r_utils/include/r_utils/r_udp_socket.h
    r_utils/source/r_udp_socket.cpp
    r_utils/include/r_utils/r_work_q.h
    r_utils/include/r_utils/r_server_threaded.h
    r_utils/include/r_utils/r_functional.h
    r_utils/include/r_utils/r_pool.h
)
target_include_directories(
    r_utils PUBLIC
    r_utils/include
)
if(CMAKE_SYSTEM MATCHES "Linux-")
    target_link_libraries(
        r_utils PUBLIC
        uuid
    )
elseif(CMAKE_SYSTEM MATCHES "Windows")
    target_link_libraries(
        r_utils PUBLIC
        DbgHelp 
        ws2_32 
        iphlpapi 
        rpcrt4.lib
    )
endif(CMAKE_SYSTEM MATCHES "Linux-")

# r_http
#
add_library(
    r_http
    r_http/include/r_http/r_client_request.h
    r_http/source/r_client_request.cpp
    r_http/include/r_http/r_client_response.h
    r_http/source/r_client_response.cpp
    r_http/include/r_http/r_http_exception.h
    r_http/source/r_http_exception.cpp
    r_http/include/r_http/r_methods.h
    r_http/source/r_methods.cpp
    r_http/include/r_http/r_range_request_generator.h
    r_http/source/r_range_request_generator.cpp
    r_http/include/r_http/r_server_request.h
    r_http/source/r_server_request.cpp
    r_http/include/r_http/r_server_response.h
    r_http/source/r_server_response.cpp
    r_http/include/r_http/r_uri.h
    r_http/source/r_uri.cpp
    r_http/include/r_http/r_utils.h
    r_http/source/r_utils.cpp
    r_http/include/r_http/r_web_server.h
    r_http/source/r_web_server.cpp
)

target_include_directories(
    r_http PUBLIC 
    r_utils/include
    r_http/include
)

target_link_libraries(
    r_http 
    r_utils
)

# r_db
#
add_library(
    r_db 
    r_db/include/r_db/r_sqlite_conn.h
    r_db/source/r_sqlite_conn.cpp
    r_db/include/r_db/r_sqlite_pager.h
    r_db/source/r_sqlite_pager.cpp
    r_db/include/sqlite3/sqlite3.h
    r_db/source/sqlite3.c
)

target_include_directories(
    r_db PUBLIC
    r_utils/include/r_utils
    r_db/include
)

target_link_libraries(
    r_db 
    r_utils
    ${CMAKE_DL_LIBS}
)

# r_mux
#
add_library(
    r_mux
    r_mux/include/r_mux/r_demuxer.h
    r_mux/source/r_demuxer.cpp
    r_mux/include/r_mux/r_muxer.h
    r_mux/source/r_muxer.cpp
    r_mux/include/r_mux/r_format_utils.h
    r_mux/source/r_format_utils.cpp
)

target_include_directories(
    r_mux PUBLIC
    r_mux/include
    r_utils/include
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_directories(
    r_mux PUBLIC
    ${FFMPEG_LIB_DIRS}
)

target_link_libraries(
    r_mux
    r_utils
    ${FFMPEG_LIBS}
)

# r_pipeline
#
add_library(
    r_pipeline 
    r_pipeline/include/r_pipeline/r_arg.h
    r_pipeline/source/r_arg.cpp
    r_pipeline/include/r_pipeline/r_gst_source.h
    r_pipeline/source/r_gst_source.cpp
    r_pipeline/include/r_pipeline/r_stream_info.h
    r_pipeline/source/r_stream_info.cpp
    r_pipeline/include/r_pipeline/r_sample_context.h
    r_pipeline/source/r_sample_context.cpp
)

target_include_directories(
    r_pipeline PUBLIC
    r_pipeline/include
    r_utils/include
    ${GSTREAMER_INCLUDE_DIRS}
)

target_link_directories(
    r_pipeline PUBLIC
    ${GSTREAMER_LIB_DIRS}
)

target_link_libraries(
    r_pipeline
    r_utils
    ${GSTREAMER_LIBS}
)

# r_fakey
#
add_library(
    r_fakey 
    r_fakey/include/r_fakey/r_fake_camera.h
    r_fakey/source/r_fake_camera.cpp
)

target_include_directories(
    r_fakey PUBLIC
    r_fakey/include
    r_utils/include
    r_mux/include
    ${GSTREAMER_INCLUDE_DIRS}
)

target_link_directories(
    r_fakey PUBLIC
    ${GSTREAMER_LIB_DIRS}
)

target_link_libraries(
    r_fakey
    r_utils
    r_mux
    ${GSTREAMER_LIBS}
)

# r_storage
#
add_library(
    r_storage 
    r_storage/include/r_storage/r_dumbdex.h
    r_storage/source/r_dumbdex.cpp
    r_storage/include/r_storage/r_ind_block.h
    r_storage/source/r_ind_block.cpp
    r_storage/include/r_storage/r_rel_block.h
    r_storage/source/r_rel_block.cpp
    r_storage/include/r_storage/r_storage_file.h
    r_storage/source/r_storage_file.cpp
)

target_include_directories(
    r_storage PUBLIC
    r_storage/include
    r_utils/include
)

target_link_libraries(
    r_storage
    r_utils
)

# r_onvif
#
add_library(
    r_onvif 
    r_onvif/include/r_onvif/r_onvif_session.h
    r_onvif/source/r_onvif_session.cpp
)

target_include_directories(
    r_onvif PUBLIC
    r_onvif/include
    r_http/include
    r_utils/include
    ${LIBXML2_INCLUDE_DIR}
)

target_link_libraries(
    r_onvif
    r_http
    r_utils
    ${LIBXML2_LIBRARIES}
)

# r_disco
#
add_library(
    r_disco 
    r_disco/include/r_disco/r_agent.h
    r_disco/source/r_agent.cpp
    r_disco/include/r_disco/r_devices.h
    r_disco/source/r_devices.cpp
    r_disco/include/r_disco/r_provider.h
    r_disco/include/r_disco/r_stream_config.h
    r_disco/include/r_disco/r_camera.h
    r_disco/source/r_stream_config.cpp
    r_disco/include/r_disco/providers/r_manual_provider.h
    r_disco/source/providers/r_manual_provider.cpp
    r_disco/include/r_disco/providers/r_onvif_provider.h
    r_disco/source/providers/r_onvif_provider.cpp
)

target_include_directories(
    r_disco PUBLIC
    r_disco/include
    r_pipeline/include
    r_utils/include
    r_db/include
    r_onvif/include
)

target_link_libraries(
    r_disco
    r_pipeline
    r_db
    r_onvif
    r_utils
)

# r_vss
#
add_library(
    r_vss 
    r_vss/include/r_vss/r_stream_keeper.h
    r_vss/source/r_stream_keeper.cpp
    r_vss/include/r_vss/r_recording_context.h
    r_vss/source/r_recording_context.cpp
)

target_include_directories(
    r_vss PUBLIC
    r_vss/include
    r_utils/include
    r_disco/include
    r_storage/include
)

target_link_libraries(
    r_vss
    r_disco
    r_storage
    r_utils
)

# silo
#

add_executable(
    silo
    silo/source/main.cpp
    silo/source/main.qml
    silo/source/Cameras.qml
    silo/source/qml.qrc
)

target_include_directories(
    silo PUBLIC 
    silo/source
    r_vss/include
    r_disco/include
    r_storage/include
    r_mux/include
    r_fakey/include
    r_pipeline/include
    r_db/include
    r_http/include
    r_utils/include
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
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)

# old_silo
#

add_executable(
    old_silo
    old_silo/source/main.cpp
    old_silo/source/mainwindow.h
    old_silo/source/mainwindow.cpp
    old_silo/source/mainwindow.ui
    old_silo/source/utils.h
    old_silo/source/utils.cpp
    old_silo/source/silo.qrc
)

target_include_directories(
    old_silo PUBLIC 
    r_vss/include
    r_disco/include
    r_storage/include
    r_mux/include
    r_fakey/include
    r_pipeline/include
    r_db/include
    r_http/include
    r_utils/include
    ${GSTREAMER_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_directories(
    old_silo PUBLIC
    ${GSTREAMER_LIB_DIRS}
    ${FFMPEG_LIB_DIRS}
)

target_link_libraries(
    old_silo LINK_PUBLIC
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

# Unit Tests
#
add_executable(
    revere_ut
    ut/include/bad_guy.h
    ut/include/framework.h
    ut/source/framework.cpp
    ut/include/utils.h
    ut/source/utils.cpp
    ut/include/test_r_utils.h
    ut/source/test_r_utils.cpp
    ut/include/test_r_http.h
    ut/source/test_r_http.cpp
    ut/include/test_r_db.h
    ut/source/test_r_db.cpp
    ut/include/test_r_pipeline.h
    ut/source/test_r_pipeline.cpp
    ut/include/test_r_mux.h
    ut/source/test_r_mux.cpp
    ut/include/test_r_storage.h
    ut/source/test_r_storage.cpp
    ut/include/test_r_disco.h
    ut/source/test_r_disco.cpp
    ut/include/test_r_vss.h
    ut/source/test_r_vss.cpp
    ut/include/test_r_onvif.h
    ut/source/test_r_onvif.cpp
)

target_include_directories(
    revere_ut PUBLIC 
    ut/include
    r_utils/include
    r_http/include
    r_db/include
    r_pipeline/include
    r_fakey/include
    r_mux/include
    r_storage/include
    r_disco/include
    r_vss/include
    r_onvif/include
    ${GSTREAMER_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)
target_link_directories(
    revere_ut PUBLIC
    ${GSTREAMER_LIB_DIRS}
    ${FFMPEG_LIB_DIRS}
)
target_link_libraries(
    revere_ut LINK_PUBLIC 
    r_http
    r_db
    r_utils
    r_pipeline
    r_fakey
    r_mux
    r_storage
    r_disco
    r_vss
    r_onvif
    ${GSTREAMER_LIBS}
    ${FFMPEG_LIBS}
)
if(CMAKE_SYSTEM MATCHES "Linux-")
    target_link_libraries(
        revere_ut PUBLIC
        pthread
        dl
        uuid
    )
elseif(CMAKE_SYSTEM MATCHES "Windows")
endif(CMAKE_SYSTEM MATCHES "Linux-")

# Tools
#
add_executable(
    fakie
    fakie/include/utils.h
    fakie/source/utils.cpp
    fakie/source/main.cpp
)
target_include_directories(
    fakie PUBLIC
    fakie/include
    r_fakey/include
)
target_link_libraries(
    fakie
    r_fakey
    r_utils
)
