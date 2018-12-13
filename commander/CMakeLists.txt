cmake_minimum_required (VERSION 2.8.12)
add_executable(commander include/assigned_devices.h
                         include/utils.h
                         source/utils.cpp
                         source/assigned_devices.cpp
                         source/main.cpp)

target_include_directories (commander PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../r_db/include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/include
                                             ${range-v3_SOURCE_DIR}/include)

target_link_libraries(commander r_http r_disco r_db r_utils pthread)
