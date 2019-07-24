cmake_minimum_required(VERSION 2.8)
include(ExternalProject)
project(ffmpeg)

set(DEFAULT_BUILD_TYPE "Debug")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}")
endif()

# --enable-omx --enable-omx-rpi --enable-mmal for raspberry pi
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  ExternalProject_Add(
    ffmpeg_linux
    URL ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-c47b48c0a6.tar.bz2
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_sources
    CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_sources/configure --pkg-config-flags="--static" --extra-ldexeflags="-Bstatic" --extra-cflags=-I${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/include --extra-cflags=-I${CMAKE_CURRENT_SOURCE_DIR}/../lame/artifacts/include --extra-ldflags=-L${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/lib --extra-ldflags=-L${CMAKE_CURRENT_SOURCE_DIR}/../lame/artifacts/lib --prefix=${CMAKE_CURRENT_SOURCE_DIR}/artifacts --enable-pic --enable-gpl --enable-static --disable-shared --enable-libx264 --enable-libmp3lame
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/artifacts
  )
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  ExternalProject_Add(
    ffmpeg_linux
    URL ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-c47b48c0a6.tar.bz2
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_sources
    CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_sources/configure --pkg-config-flags="--static" --extra-ldexeflags="-Bstatic" --extra-cflags=-I${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/include --extra-cflags=-I${CMAKE_CURRENT_SOURCE_DIR}/../lame/artifacts/include --extra-ldflags=-L${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/lib --extra-ldflags=-L${CMAKE_CURRENT_SOURCE_DIR}/../lame/artifacts/lib --prefix=${CMAKE_CURRENT_SOURCE_DIR}/artifacts --enable-pic --enable-gpl --enable-static --disable-shared --enable-gpl --enable-libx264 --enable-libmp3lame --enable-pthreads --enable-runtime-cpudetect --disable-doc
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/artifacts
  )
endif(CMAKE_BUILD_TYPE STREQUAL "Release")
