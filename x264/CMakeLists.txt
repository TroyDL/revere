cmake_minimum_required(VERSION 2.8)
include(ExternalProject)
project(x264)

ExternalProject_Add(
    x264_linux
    URL ${CMAKE_CURRENT_SOURCE_DIR}/x264.tar.bz2
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/x264_sources
    CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/x264_sources/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/artifacts --enable-shared --disable-avs --disable-cli --disable-gpl --disable-opencl
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/artifacts
)
