cmake_minimum_required (VERSION 2.8.12)

add_library (r_disco include/r_disco/r_discovery.h
                     include/r_disco/r_recognizer.h
                     include/r_disco/interfaces/r_query_device_info.h
                     include/r_disco/r_cirrus_device_info_agent.h
                     source/r_discovery.cpp
                     source/r_recognizer.cpp
                     source/r_device_info.cpp
                     source/r_cirrus_device_info_agent.cpp)

target_include_directories (r_disco PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                           ${CMAKE_CURRENT_SOURCE_DIR}/../r_utils/include
                                           ${CMAKE_CURRENT_SOURCE_DIR}/../r_http/include)

target_link_libraries (r_disco r_utils r_http)

#add_subdirectory (ut)
