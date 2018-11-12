cmake_minimum_required (VERSION 2.8.12)

add_executable (r_av_ut include/framework.h
                        include/test_r_av_r_demuxer.h
                        include/test_r_av_r_muxer.h
                        include/test_r_av_r_video_decoder.h
                        include/test_r_av_r_video_encoder.h
                        include/test_r_av_r_transcoder.h
                        source/framework.cpp
                        source/test_r_av_r_demuxer.cpp
                        source/test_r_av_r_muxer.cpp
                        source/test_r_av_r_video_decoder.cpp
                        source/test_r_av_r_video_encoder.cpp
                        source/test_r_av_r_transcoder.cpp)

target_include_directories (r_av_ut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                           ${CMAKE_CURRENT_SOURCE_DIR}/../include)

find_library(X264_LIB NAMES libx264.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/lib)
find_library(FFMPEG_LIBSWSCALE NAMES libswscale.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVDEVICE libavdevice,so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVFORMAT NAMES libavformat.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVCODEC NAMES libavcodec.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVUTIL NAMES libavutil.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVFILTER NAMES libavfilter.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg/artifacts/lib)
                                           
target_link_libraries (r_av_ut LINK_PUBLIC r_av ${FFMPEG_LIBSWSCALE} ${FFMPEG_LIBAVDEVICE} ${FFMPEG_LIBAVFORMAT} ${FFMPEG_LIBAVCODEC} ${FFMPEG_LIBAVUTIL} ${FFMPEG_LIBAVFILTER} ${X264_LIB} r_utils)
