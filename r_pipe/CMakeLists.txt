cmake_minimum_required (VERSION 2.8.12)

add_library (r_pipe include/r_pipe/r_control.h
                    include/r_pipe/r_filter_group.h
                    include/r_pipe/r_filter.h
                    include/r_pipe/r_media_object.h
                    include/r_pipe/r_source.h
                    include/r_pipe/r_stateful.h
                    include/r_pipe/plugins/rtsp_source/r_rtsp_source.h
                    include/r_pipe/plugins/storage_sink/r_storage_sink.h
                    source/r_media_object.cpp
                    source/r_control.cpp
                    source/plugins/rtsp_source/r_rtsp_source.cpp
                    source/plugins/storage_sink/r_storage_sink.cpp)

target_include_directories (r_pipe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                          ${CMAKE_CURRENT_SOURCE_DIR}/../r_utils/include
                                          ${CMAKE_CURRENT_SOURCE_DIR}/../r_av/include
                                          ${CMAKE_CURRENT_SOURCE_DIR}/../r_rtsp/include
                                          ${CMAKE_CURRENT_SOURCE_DIR}/../r_storage/include)

target_link_libraries (r_pipe r_utils r_av r_rtsp r_storage)

#add_subdirectory (ut)
