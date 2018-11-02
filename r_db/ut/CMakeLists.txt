cmake_minimum_required (VERSION 2.8.12)

add_executable (r_db_ut include/framework.h
                        include/test_r_db_r_sqlite_conn.h
                        include/test_r_db_r_sqlite_pager.h
                        source/framework.cpp
                        source/test_r_db_r_sqlite_conn.cpp
                        source/test_r_db_r_sqlite_pager.cpp)

target_include_directories (r_db_ut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                           ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries (r_db_ut LINK_PUBLIC r_db r_utils)
