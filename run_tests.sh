#!/bin/bash
valgrind build/r_utils/ut/r_utils_ut
valgrind build/r_http/ut/r_http_ut
valgrind build/r_av/ut/r_av_ut
valgrind build/r_rtsp/ut/r_rtsp_ut
valgrind build/r_db/ut/r_db_ut
valgrind build/r_storage/ut/r_storage_ut
valgrind build/r_vss_client/ut/r_vss_client_ut
