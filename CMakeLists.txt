cmake_minimum_required(VERSION 3.20)

project(Omity)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(Omity
	src/engine_core.cpp
	src/engine_minify.cpp
	src/logger.cpp
	src/main.cpp
   "src/string_utils.cpp" "include/file_event.h")