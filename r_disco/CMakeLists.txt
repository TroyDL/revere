cmake_minimum_required (VERSION 2.8.12)

add_library (r_disco include/r_disco/r_discovery.h
                     source/r_discovery.cpp)

target_include_directories (r_disco PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                           ${CMAKE_CURRENT_SOURCE_DIR}/../r_utils/include
                                           ${CMAKE_CURRENT_SOURCE_DIR}/../r_http/include)

target_link_libraries (r_disco r_utils r_http)

#add_subdirectory (ut)
