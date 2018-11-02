cmake_minimum_required (VERSION 2.8.12)

add_executable (r_http_ut include/framework.h
                          include/test_r_http_r_client_request.h
                          include/test_r_http_r_client_response.h
                          include/test_r_http_r_range_request_generator.h
                          include/test_r_http_r_server_response.h
                          include/test_r_http_r_utils.h
                          include/test_r_http_r_web_server.h
                          source/framework.cpp
                          source/test_r_http_r_client_request.cpp
                          source/test_r_http_r_client_response.cpp
                          source/test_r_http_r_range_request_generator.cpp
                          source/test_r_http_r_server_response.cpp
                          source/test_r_http_r_utils.cpp
                          source/test_r_http_r_web_server.cpp)

target_include_directories (r_http_ut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries (r_http_ut LINK_PUBLIC r_http r_utils)
