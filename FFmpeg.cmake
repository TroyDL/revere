
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(
    FFMPEG_INCLUDE_DIRS
    ../deps/windows/ffmpeg/include
)

set(
    FFMPEG_LIB_DIRS
    ../deps/windows/ffmpeg/lib
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
