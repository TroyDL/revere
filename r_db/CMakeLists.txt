cmake_minimum_required (VERSION 2.8.12)

add_library (r_db include/r_db/r_sqlite_conn.h
                  include/r_db/r_sqlite_pager.h
                  include/sqlite3/sqlite3.h
                  source/r_sqlite_conn.cpp
                  source/r_sqlite_pager.cpp
                  source/sqlite3.c)

target_include_directories (r_db PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../r_utils/include
                                        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries (r_db r_utils)

add_subdirectory (ut)