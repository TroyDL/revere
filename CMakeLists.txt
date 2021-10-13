cmake_minimum_required(VERSION 2.8.12)
project(revere)

# Global Config

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_compile_definitions(IS_WINDOWS)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_compile_definitions(IS_LINUX)
endif()

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

set(
    GSTREAMER_INCLUDE_DIRS
    deps/linux/gstreamer/1.0/gcc_x86_64/include/gstreamer-1.0
    /usr/include/glib-2.0
    /usr/lib/x86_64-linux-gnu/glib-2.0/include/
)

set(
    GSTREAMER_LIB_DIRS
    deps/linux/gstreamer/1.0/gcc_x86_64/lib/x86_64-linux-gnu
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

set(
    FFMPEG_INCLUDE_DIRS
    deps/linux/ffmpeg/include
)

set(
    FFMPEG_LIB_DIRS
    deps/linux/ffmpeg/lib
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

# Libraries
#

# r_utils
#
add_library(
    r_utils
    r_utils/include/r_utils/r_actor.h
    r_utils/include/r_utils/r_args.h
    r_utils/include/r_utils/r_macro.h
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
)

target_include_directories(
    r_disco PUBLIC
    r_disco/include
    r_pipeline/include
    r_utils/include
    r_db/include
)

target_link_libraries(
    r_disco
    r_pipeline
    r_utils
    r_db
)

# Ok, We need a lib that wraps our pipeline
#     - Parameters
#       - rtsp url
#       - username
#       - password
#     - Callback for arriving frames (audio and video)
#     - Sync audio and video

# Unit Tests
#
add_executable(
    revere_ut
    ut/include/bad_guy.h
    ut/include/framework.h
    ut/source/framework.cpp
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
    r_utils
    r_fakey
)
