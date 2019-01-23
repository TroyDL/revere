cmake_minimum_required (VERSION 2.8.12)
add_executable(vas source/main.cpp
                   source/motion_context.cpp)

target_include_directories (vas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                       ${range-v3_SOURCE_DIR}/include
                                       ${CMAKE_CURRENT_SOURCE_DIR}/../r_vss_client/include)

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
find_library(MMALCORE_LIB NAMES mmal_core PATHS /opt/vc/lib)
find_library(MMALUTIL_LIB NAMES mmal_util PATHS /opt/vc/lib)
find_library(MMAL_LIB NAMES mmal PATHS /opt/vc/lib)
find_library(MMALCOMPONENTS_LIB NAMES mmal_components PATHS /opt/vc/lib)
find_library(MMALVCCLIENT_LIB NAMES mmal_vc_client PATHS /opt/vc/lib)
find_library(BCMHOST_LIB NAMES bcm_host PATHS /opt/vc/lib)

target_link_libraries(vas r_av ${FFMPEG_LIBSWSCALE} ${FFMPEG_LIBAVDEVICE} ${FFMPEG_LIBAVFORMAT} ${FFMPEG_LIBAVFILTER} ${FFMPEG_LIBPOSTPROC} ${FFMPEG_LIBSWSCALE} ${FFMPEG_LIBAVCODEC} ${MMALVCCLIENT_LIB} ${MMAL_LIB} ${MMALCORE_LIB} ${MMALUTIL_LIB} ${MMALCOMPONENTS_LIB} ${BCMHOST_LIB} ${FFMPEG_LIBSWRESAMPLE} ${FFMPEG_LIBAVUTIL} ${LAME_LIBRARIES} z ${BZ2_LIBRARIES} ${X264_LIB} r_vss_client r_utils)
