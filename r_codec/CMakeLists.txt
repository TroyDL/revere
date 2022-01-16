
add_library(
    r_codec
    include/r_codec/r_video_decoder.h
    source/r_video_decoder.cpp
)

target_include_directories(
    r_codec PUBLIC
    include
    ../r_utils/include
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_directories(
    r_codec PUBLIC
    ${FFMPEG_LIB_DIRS}
)

target_link_libraries(
    r_codec
    r_utils
    ${FFMPEG_LIBS}
)
