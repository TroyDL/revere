cmake_minimum_required(VERSION 2.8.12)
project(revere)
include(TestBigEndian)

# Global Config

if(IS_BIG_ENDIAN)
    add_compile_definitions(BYTE_ORDER=BIG_ENDIAN)
else()
    add_compile_definitions(BYTE_ORDER=LITTLE_ENDIAN)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Debug)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_compile_definitions(IS_WINDOWS)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_compile_definitions(IS_LINUX)
endif()

include(Qt.cmake)
include(GStreamer.cmake)
include(FFmpeg.cmake)

find_package(Threads)
find_package(LibXml2)

#find_package(PkgConfig REQUIRED)
#pkg_search_module(GLFW REQUIRED glfw3)
#find_package(OpenGL REQUIRED)

# Libraries
#
add_subdirectory(r_utils)
add_subdirectory(r_db)
add_subdirectory(r_http)
add_subdirectory(r_mux)
add_subdirectory(r_codec)
add_subdirectory(r_pipeline)
add_subdirectory(r_fakey)
add_subdirectory(r_storage)
add_subdirectory(r_onvif)
add_subdirectory(r_disco)
add_subdirectory(r_motion)
add_subdirectory(r_vss)

# Applications
#
add_subdirectory(silo)

# Unit Tests
#
add_subdirectory(ut)

# Tools
#
add_subdirectory(fakie)
