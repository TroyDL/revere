cmake_minimum_required (VERSION 2.8.12)
project (revere)

if(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
endif()

set (CMAKE_CXX_FLAGS_DEBUG "-std=c++14 -pthread -Wall -Wfatal-errors -O0 -ggdb")
set (CMAKE_CXX_FLAGS_RELEASE "-std=c++14 -pthread -Wall -Wfatal-errors -O3")

# libs
add_subdirectory (bento4)
add_subdirectory (r_utils)
add_subdirectory (r_db)
add_subdirectory (r_storage)
add_subdirectory (r_http)
add_subdirectory (r_disco)
add_subdirectory (r_av)
add_subdirectory (r_rtsp)
add_subdirectory (r_pipe)
add_subdirectory (r_vss_client)

# services
add_subdirectory (vss) # video storage service
add_subdirectory (vts) # video transcode service
add_subdirectory (fakie) # fake camera
add_subdirectory (test)