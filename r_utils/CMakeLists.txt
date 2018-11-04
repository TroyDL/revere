cmake_minimum_required (VERSION 2.8.12)

add_library (r_utils include/r_utils/r_string_utils.h
                     include/r_utils/r_logger.h
                     include/r_utils/r_stack_trace.h
                     include/r_utils/r_socket_address.h
                     include/r_utils/r_sha_256.h
                     include/r_utils/r_socket.h
                     include/r_utils/r_exception.h
                     include/r_utils/r_nullable.h
                     include/r_utils/r_file.h
                     include/r_utils/r_timer.h
                     include/r_utils/r_byte_ptr.h
                     include/r_utils/r_udp_receiver.h
                     include/r_utils/r_udp_sender.h
                     include/r_utils/r_pool.h
                     include/r_utils/r_memory_map.h
                     include/r_utils/r_uuid.h
                     include/r_utils/r_file_lock.h
                     include/r_utils/r_algorithms.h
                     include/r_utils/r_time_utils.h
                     include/r_utils/r_functional.h
                     include/r_utils/r_blob_tree.h
                     include/r_utils/r_process.h
                     include/r_utils/r_server_threaded.h # header only
                     source/r_string_utils.cpp
                     source/r_logger.cpp
                     source/r_stack_trace.cpp
                     source/r_socket_address.cpp
                     source/r_sha_256.cpp
                     source/r_socket.cpp
                     source/r_exception.cpp
                     source/r_nullable.cpp
                     source/r_file.cpp
                     source/r_timer.cpp
                     source/r_byte_ptr.cpp
                     source/r_udp_receiver.cpp
                     source/r_udp_sender.cpp
                     source/r_pool.cpp
                     source/r_memory_map.cpp
                     source/r_uuid.cpp
                     source/r_file_lock.cpp
                     source/r_algorithms.cpp
                     source/r_blob_tree.cpp
                     source/r_time_utils.cpp
                     source/r_process.cpp)

target_include_directories (r_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_library(SSL_LIBRARY ssl)
find_library(CRYPTO_LIBRARY crypto)
find_library(DL_LIBRARY dl)
find_library(PTHREAD_LIBRARY pthread)
find_library(UUID_LIBRARY uuid)

target_link_libraries (r_utils ${PTHREAD_LIBRARY} ${SSL_LIBRARY} ${CRYPTO_LIBRARY} ${DL_LIBRARY} ${UUID_LIBRARY})

add_subdirectory (ut)