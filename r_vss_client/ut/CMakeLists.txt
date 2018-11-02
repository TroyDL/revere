cmake_minimum_required (VERSION 2.8.12)

add_executable (r_vss_client_ut include/framework.h
                                include/test_r_vss_client_r_media_parser.h
                                include/test_r_vss_client_r_query_generator.h
                                source/framework.cpp
                                source/test_r_vss_client_r_media_parser.cpp
                                source/test_r_vss_client_r_query_generator.cpp)

target_include_directories (r_vss_client_ut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries (r_vss_client_ut LINK_PUBLIC r_utils r_vss_client)