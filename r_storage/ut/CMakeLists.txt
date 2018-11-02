cmake_minimum_required (VERSION 2.8.12)

add_executable (r_storage_ut include/framework.h
                            include/test_r_storage_r_append_file.h
                            include/test_r_storage_r_file_index.h
                            source/framework.cpp
                            source/test_r_storage_r_append_file.cpp
                            source/test_r_storage_r_file_index.cpp)

target_include_directories (r_storage_ut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries (r_storage_ut LINK_PUBLIC r_storage r_db r_utils)
