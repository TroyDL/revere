cmake_minimum_required (VERSION 2.8.12)

add_library (r_av include/r_av/r_packet.h
                  include/r_av/r_packet_factory.h
                  include/r_av/r_demuxer.h
                  include/r_av/r_options.h
                  include/r_av/r_muxer.h
                  include/r_av/r_locky.h
                  include/r_av/r_video_decoder.h
                  include/r_av/r_utils.h
                  include/r_av/r_video_encoder.h
                  include/r_av/r_transcoder.h
                  source/r_packet.cpp
                  source/r_packet_factory.cpp
                  source/r_demuxer.cpp
                  source/r_options.cpp
                  source/r_muxer.cpp
                  source/r_locky.cpp
                  source/r_video_decoder.cpp
                  source/r_utils.cpp
                  source/r_video_encoder.cpp)

target_include_directories (r_av PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../r_utils/include
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/include
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../bento4/Source/C++/Core
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../bento4/Source/C++/Codecs
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../bento4/Source/C++/Crypto
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../bento4/Source/C++/MetaData
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../bento4/Source/C++/CApi
                                        ${CMAKE_CURRENT_SOURCE_DIR}/include)

#find_library(X264_LIB NAMES libx264.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/lib)
#find_library(FFMPEG_LIBSWSCALE NAMES libswscale.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
#find_library(FFMPEG_LIBAVDEVICE libavdevice.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
#find_library(FFMPEG_LIBAVFORMAT NAMES libavformat.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
#find_library(FFMPEG_LIBAVCODEC NAMES libavcodec.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
#find_library(FFMPEG_LIBAVUTIL NAMES libavutil.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
#find_library(FFMPEG_LIBAVFILTER NAMES libavfilter.so PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)

find_library(X264_LIB NAMES libx264.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/lib)
find_library(FFMPEG_LIBSWSCALE NAMES libswscale.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVDEVICE libavdevice.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVFORMAT NAMES libavformat.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVCODEC NAMES libavcodec.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVUTIL NAMES libavutil.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
find_library(FFMPEG_LIBAVFILTER NAMES libavfilter.a PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg/artifacts/lib)
find_library(LAME_LIBRARIES NAMES libmp3lame.a)
find_library(BZ2_LIBRARIES NAMES libbz2.a)

target_link_libraries (r_av ${FFMPEG_LIBSWSCALE} ${FFMPEG_LIBAVDEVICE} ${FFMPEG_LIBAVFORMAT} ${FFMPEG_LIBAVCODEC} ${FFMPEG_LIBAVUTIL} ${FFMPEG_LIBAVFILTER} ${X264_LIB} r_utils)

add_subdirectory (ut)
