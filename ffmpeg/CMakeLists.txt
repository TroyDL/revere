cmake_minimum_required(VERSION 2.8)
include(ExternalProject)
project(ffmpeg)

set(DEFAULT_BUILD_TYPE "Debug")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  ExternalProject_Add(
    ffmpeg_linux
    URL ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-3.4.1.tar.bz2
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_sources
    CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_sources/configure --extra-cflags=-I${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/include --extra-ldflags=-L${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/lib --prefix=${CMAKE_CURRENT_SOURCE_DIR}/artifacts --enable-gpl --enable-libx264 --enable-shared --disable-static --enable-pthreads --disable-optimizations --disable-mmx --disable-stripping --enable-vaapi --enable-hwaccel=h264_vaapi --enable-libmp3lame --disable-programs --disable-doc
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/artifacts
  )
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  ExternalProject_Add(
    ffmpeg_linux
    URL ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-3.4.1.tar.bz2
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_sources
    CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_sources/configure --extra-cflags=-I${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/include --extra-ldflags=-L${CMAKE_CURRENT_SOURCE_DIR}/../x264/artifacts/lib --prefix=${CMAKE_CURRENT_SOURCE_DIR}/artifacts --enable-gpl --enable-libx264 --enable-shared --disable-static --enable-pthreads --enable-runtime-cpudetect --enable-vaapi --enable-hwaccel=h264_vaapi --enable-libmp3lame --disable-programs --disable-doc
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/artifacts
  )
endif(CMAKE_BUILD_TYPE STREQUAL "Release")
