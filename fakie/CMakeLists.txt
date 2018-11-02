cmake_minimum_required (VERSION 2.8.12)

add_executable(fakie source/main.cpp)
target_link_libraries(fakie r_rtsp r_av r_http r_utils pthread)
