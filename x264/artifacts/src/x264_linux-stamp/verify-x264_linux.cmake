# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/home/td/code/dicroce/revere/x264/x264.tar.bz2" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/home/td/code/dicroce/revere/x264/x264.tar.bz2")
  message(FATAL_ERROR "File not found: /home/td/code/dicroce/revere/x264/x264.tar.bz2")
endif()

if("" STREQUAL "")
  message(WARNING "File will not be verified since no URL_HASH specified")
  return()
endif()

if("" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(STATUS "verifying file...
     file='/home/td/code/dicroce/revere/x264/x264.tar.bz2'")

file("" "/home/td/code/dicroce/revere/x264/x264.tar.bz2" actual_value)

if(NOT "${actual_value}" STREQUAL "")
  message(FATAL_ERROR "error:  hash of
  /home/td/code/dicroce/revere/x264/x264.tar.bz2
does not match expected value
  expected: ''
    actual: '${actual_value}'
")
endif()

message(STATUS "verifying file... done")
