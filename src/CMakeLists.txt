cmake_minimum_required(VERSION 2.8)

project(mod_tree)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_BUILD_TYPE "Debug")
include_directories(./include)

add_executable(mod_treed
			src/db_mysql.cpp
			src/common.cpp
			src/main.cpp)
target_link_libraries(mod_treed c rt  pthread  mysqlclient  boost_system boost_program_options)

