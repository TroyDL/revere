cmake_minimum_required (VERSION 2.8.12)

add_executable (r_utils_ut include/framework.h
                           include/test_r_utils_r_string.h
                           include/test_r_utils_r_sha_256.h
                           include/test_r_utils_r_socket.h
                           include/test_r_utils_r_server_threaded.h
                           include/test_r_utils_r_byte_ptr.h
                           include/test_r_utils_r_file.h
                           include/test_r_utils_r_timer.h
                           include/test_r_utils_r_udp_receiver.h
                           include/test_r_utils_r_uuid.h
                           include/test_r_utils_r_file_lock.h
                           include/test_r_utils_r_time_utils.h
                           include/test_r_utils_r_blob_tree.h
                           source/framework.cpp
                           source/test_r_utils_r_string.cpp
                           source/test_r_utils_r_sha_256.cpp
                           source/test_r_utils_r_socket.cpp
                           source/test_r_utils_r_server_threaded.cpp
                           source/test_r_utils_r_byte_ptr.cpp
                           source/test_r_utils_r_file.cpp
                           source/test_r_utils_r_timer.cpp
                           source/test_r_utils_r_udp_receiver.cpp
                           source/test_r_utils_r_uuid.cpp
                           source/test_r_utils_r_file_lock.cpp
                           source/test_r_utils_r_time_utils.cpp
                           source/test_r_utils_r_blob_tree.cpp)

target_include_directories (r_utils_ut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries (r_utils_ut LINK_PUBLIC r_utils)