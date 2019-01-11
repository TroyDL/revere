cmake_minimum_required (VERSION 2.8.12)

add_library (r_rtsp include/r_rtsp/r_client_connection.h
                    include/r_rtsp/r_client_request.h
                    include/r_rtsp/r_client_response.h
                    include/r_rtsp/r_methods.h
                    include/r_rtsp/r_exception.h
                    include/r_rtsp/r_rtp.h
                    include/r_rtsp/r_rtsp_server.h
                    include/r_rtsp/r_server_connection.h
                    include/r_rtsp/r_server_request.h
                    include/r_rtsp/r_server_response.h
                    include/r_rtsp/r_session_base.h
                    include/r_rtsp/r_status.h
                    include/r_rtsp/r_fake_camera.h
                    include/r_rtsp/r_sdp.h
                    source/r_client_connection.cpp
                    source/r_client_request.cpp
                    source/r_client_response.cpp
                    source/r_methods.cpp
                    source/r_exception.cpp
                    source/r_rtp.cpp
                    source/r_rtsp_server.cpp
                    source/r_server_connection.cpp
                    source/r_server_request.cpp
                    source/r_server_response.cpp
                    source/r_status.cpp
                    source/r_fake_camera.cpp
                    source/r_sdp.cpp)

target_include_directories (r_rtsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../r_utils/include
                                          ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_library(X264_LIB NAMES libx264.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/lib)
find_library(FFMPEG_LIBSWSCALE NAMES libswscale.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBSWRESAMPLE NAMES libswresample.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVDEVICE libavdevice.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVFORMAT NAMES libavformat.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVCODEC NAMES libavcodec.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVUTIL NAMES libavutil.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVFILTER NAMES libavfilter.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBPOSTPROC NAMES libpostproc.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(LAME_LIBRARIES NAMES libmp3lame.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../lame/artifacts/lib)
find_library(BZ2_LIBRARIES NAMES libbz2.a)

target_link_libraries (r_rtsp r_utils r_av ${FFMPEG_LIBAVDEVICE} ${FFMPEG_LIBAVFORMAT} ${FFMPEG_LIBAVFILTER} ${FFMPEG_LIBSWSCALE} ${FFMPEG_LIBPOSTPROC} ${FFMPEG_LIBSWSCALE} ${FFMPEG_LIBAVCODEC} ${FFMPEG_LIBSWRESAMPLE} ${FFMPEG_LIBAVUTIL} ${LAME_LIBRARIES} z ${BZ2_LIBRARIES} ${X264_LIB})

add_subdirectory (ut)
