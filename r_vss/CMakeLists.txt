
# r_vss
#
add_library(
    r_vss 
    include/r_vss/r_stream_keeper.h
    source/r_stream_keeper.cpp
    include/r_vss/r_recording_context.h
    source/r_recording_context.cpp
)

target_include_directories(
    r_vss PUBLIC
    include
    ../r_utils/include
    ../r_disco/include
    ../r_storage/include
)

target_link_libraries(
    r_vss
    r_disco
    r_storage
    r_utils
)
