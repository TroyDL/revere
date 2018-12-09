cmake_minimum_required (VERSION 2.8.12)
add_executable(commander source/main.cpp)

target_include_directories (commander PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(commander r_http r_disco r_utils pthread)
