cmake_minimum_required (VERSION 2.8.12)

add_executable (r_rtsp_ut include/framework.h
                          include/test_r_rtsp_r_client_connection.h
                          include/test_r_rtsp_r_client_request.h
                          include/test_r_rtsp_r_rtp.h
                          include/test_r_rtsp_r_rtsp_server.h
                          include/test_r_rtsp_r_server_response.h
                          include/test_r_rtsp_r_fake_camera.h
                          include/test_r_rtsp_r_sdp.h
                          source/framework.cpp
                          source/test_r_rtsp_r_client_connection.cpp
                          source/test_r_rtsp_r_client_request.cpp
                          source/test_r_rtsp_r_rtp.cpp
                          source/test_r_rtsp_r_rtsp_server.cpp
                          source/test_r_rtsp_r_server_response.cpp
                          source/test_r_rtsp_r_fake_camera.cpp
                          source/test_r_rtsp_r_sdp.cpp)

target_include_directories (r_rtsp_ut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries (r_rtsp_ut LINK_PUBLIC r_rtsp r_utils)
