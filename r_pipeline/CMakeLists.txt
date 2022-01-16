
add_library(
    r_pipeline 
    include/r_pipeline/r_arg.h
    source/r_arg.cpp
    include/r_pipeline/r_gst_source.h
    source/r_gst_source.cpp
    include/r_pipeline/r_stream_info.h
    source/r_stream_info.cpp
    include/r_pipeline/r_sample_context.h
    source/r_sample_context.cpp
)

target_include_directories(
    r_pipeline PUBLIC
    include
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
