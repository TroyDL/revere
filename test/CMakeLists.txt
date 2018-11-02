cmake_minimum_required (VERSION 2.8.12)
add_executable(tst source/main.cpp)

target_include_directories (tst PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(tst r_utils pthread)
