cmake_minimum_required(VERSION 2.8)
include(ExternalProject)
project(lame)

ExternalProject_Add(
    lame_linux
    URL ${CMAKE_CURRENT_SOURCE_DIR}/lame-3.100.tar.gz
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/lame_sources
    CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/lame_sources/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/artifacts --enable-static
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/artifacts
)
