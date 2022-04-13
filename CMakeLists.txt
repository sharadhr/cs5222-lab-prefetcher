cmake_minimum_required(VERSION 3.16)

project(
	cs5222_prefetcher
	VERSION 1.0
	DESCRIPTION "CS5222 Project 3: Prefetchers"
	LANGUAGES CXX)

# C++20 required, disable extensions, add error flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -no-pie")

add_subdirectory(src)

target_include_directories(dpc2sim PRIVATE inc)
find_library(dpclib dpc2sim "${CMAKE_SOURCE_DIR}/lib")
target_link_libraries(dpc2sim ${dpclib})
#target_compile_definitions()
