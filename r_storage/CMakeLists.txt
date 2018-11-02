cmake_minimum_required (VERSION 2.8.12)

add_library (r_storage include/r_storage/r_append_file.h
                       include/r_storage/r_file_index.h
                       include/r_storage/r_storage_engine.h
                       include/r_storage/r_segment_file.h
                       include/r_storage/r_stream_time.h
                       include/r_storage/r_queries.h
                       source/r_append_file.cpp
                       source/r_file_index.cpp
                       source/r_storage_engine.cpp
                       source/r_stream_time.cpp
                       source/r_queries.cpp)


target_include_directories (r_storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../r_utils/include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/../r_db/include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries (r_storage r_db r_utils)

add_subdirectory (ut)
