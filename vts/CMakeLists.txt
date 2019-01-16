cmake_minimum_required (VERSION 2.8.12)
add_executable(vts include/video.h
                   include/jpg.h
                   include/stream_segment.h
                   source/video.cpp
                   source/jpg.cpp
                   source/stream_segment.cpp
                   source/main.cpp)
                   
target_include_directories (vts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(vts r_rtsp r_av r_vss_client r_http r_utils pthread)
