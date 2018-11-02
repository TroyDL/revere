cmake_minimum_required (VERSION 2.8.12)

add_library (r_vss_client include/r_vss_client/r_frame.h
                          include/r_vss_client/r_media_parser.h
                          include/r_vss_client/r_query.h
                          include/r_vss_client/r_query_generator.h
                          include/r_vss_client/r_sdp.h
                          source/r_frame.cpp
                          source/r_media_parser.cpp
                          source/r_query.cpp
                          source/r_query_generator.cpp
                          source/r_sdp.cpp)

target_include_directories (r_vss_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) 

target_link_libraries (r_vss_client r_utils r_http)

add_subdirectory (ut)
