cmake_minimum_required (VERSION 2.8.12)

add_library (r_http include/r_http/r_client_request.h
                    include/r_http/r_client_response.h
                    include/r_http/r_http_exception.h
                    include/r_http/r_methods.h
                    include/r_http/r_range_request_generator.h
                    include/r_http/r_server_request.h
                    include/r_http/r_server_response.h
                    include/r_http/r_uri.h
                    include/r_http/r_utils.h
                    include/r_http/r_web_server.h
                    source/r_client_request.cpp
                    source/r_client_response.cpp
                    source/r_http_exception.cpp
                    source/r_methods.cpp
                    source/r_range_request_generator.cpp
                    source/r_server_request.cpp
                    source/r_server_response.cpp
                    source/r_uri.cpp
                    source/r_utils.cpp
                    source/r_web_server.cpp)


target_include_directories (r_http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../r_utils/include
                                          ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries (r_http r_utils)

add_subdirectory (ut)