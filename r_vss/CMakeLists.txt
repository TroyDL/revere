
# r_vss
#
add_library(
    r_vss 
    include/r_vss/r_stream_keeper.h
    source/r_stream_keeper.cpp
    include/r_vss/r_recording_context.h
    source/r_recording_context.cpp
    include/r_vss/r_motion_engine.h
    source/r_motion_engine.cpp
)

target_include_directories(
    r_vss PUBLIC
    include
    ../r_motion/include
    ../r_codec/include
    ../r_mux/include
    ../r_disco/include
    ../r_storage/include
    ../r_utils/include
    ${GSTREAMER_INCLUDE_DIRS}
)

target_link_directories(
    r_vss PUBLIC
    ${GSTREAMER_LIB_DIRS}
)

target_link_libraries(
    r_vss
    r_motion
    r_codec
    r_mux
    r_disco
    r_storage
    r_utils
    ${GSTREAMER_LIBS}
)
