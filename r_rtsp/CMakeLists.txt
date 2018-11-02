cmake_minimum_required (VERSION 2.8.12)

add_library (r_rtsp include/r_rtsp/r_client_connection.h
                    include/r_rtsp/r_client_request.h
                    include/r_rtsp/r_client_response.h
                    include/r_rtsp/r_methods.h
                    include/r_rtsp/r_exception.h
                    include/r_rtsp/r_rtp.h
                    include/r_rtsp/r_rtsp_server.h
                    include/r_rtsp/r_server_connection.h
                    include/r_rtsp/r_server_request.h
                    include/r_rtsp/r_server_response.h
                    include/r_rtsp/r_session_base.h
                    include/r_rtsp/r_status.h
                    include/r_rtsp/r_fake_camera.h
                    include/r_rtsp/r_sdp.h
                    source/r_client_connection.cpp
                    source/r_client_request.cpp
                    source/r_client_response.cpp
                    source/r_methods.cpp
                    source/r_exception.cpp
                    source/r_rtp.cpp
                    source/r_rtsp_server.cpp
                    source/r_server_connection.cpp
                    source/r_server_request.cpp
                    source/r_server_response.cpp
                    source/r_status.cpp
                    source/r_fake_camera.cpp
                    source/r_sdp.cpp)

target_include_directories (r_rtsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../r_utils/include
                                          ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries (r_rtsp r_utils r_av)

add_subdirectory (ut)