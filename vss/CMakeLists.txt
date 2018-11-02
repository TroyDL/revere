cmake_minimum_required (VERSION 2.8.12)
add_executable(vss include/data_sources.h
                   source/data_sources.cpp
                   source/main.cpp)

target_include_directories (vss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(vss r_pipe r_storage r_http r_utils pthread)
