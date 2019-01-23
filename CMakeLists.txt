cmake_minimum_required (VERSION 2.8.12)
project (revere)

include(FetchContent)

FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3
    GIT_TAG master
)

FetchContent_GetProperties(range-v3)
if(NOT range-v3_POPULATED)
  FetchContent_Populate(range-v3)
  add_subdirectory(${range-v3_SOURCE_DIR} ${range-v3_BINARY_DIR})
endif()

if(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
endif()

set (CMAKE_CXX_FLAGS_DEBUG "-std=c++14 -pthread -Wall -Wfatal-errors -O0 -ggdb")
set (CMAKE_CXX_FLAGS_RELEASE "-std=c++14 -pthread -Wall -Wfatal-errors -O3")

# libs
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
add_subdirectory (vas) # video analytics service
add_subdirectory (commander)
add_subdirectory (fakie) # fake camera
add_subdirectory (test)
