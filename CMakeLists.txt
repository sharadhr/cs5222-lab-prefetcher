cmake_minimum_required(VERSION 3.16)

project(
	cs5222_prefetcher
	VERSION 1.0
	DESCRIPTION "CS5222 Project 3: Prefetchers"
	LANGUAGES CXX)

# C++20 required, disable extensions, add error flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -no-pie")

add_subdirectory(src)
add_subdirectory(example_prefetchers)
find_library(dpclib dpc2sim "${CMAKE_SOURCE_DIR}/lib")

target_include_directories(ghb-pc-dc PRIVATE inc)
target_include_directories(ampm-lite PRIVATE inc)
target_include_directories(ip-stride PRIVATE inc)
target_include_directories(next-line PRIVATE inc)
target_include_directories(skeleton PRIVATE inc)
target_include_directories(stream PRIVATE inc)

target_link_libraries(ghb-pc-dc ${dpclib})
target_link_libraries(ampm-lite ${dpclib})
target_link_libraries(ip-stride ${dpclib})
target_link_libraries(next-line ${dpclib})
target_link_libraries(skeleton ${dpclib})
target_link_libraries(stream ${dpclib})

set_target_properties(ghb-pc-dc PROPERTIES LINKER_LANGUAGE CXX RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/$<0:>)
set_target_properties(ampm-lite PROPERTIES LINKER_LANGUAGE CXX RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/$<0:>)
set_target_properties(ip-stride PROPERTIES LINKER_LANGUAGE CXX RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/$<0:>)
set_target_properties(next-line PROPERTIES LINKER_LANGUAGE CXX RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/$<0:>)
set_target_properties(skeleton PROPERTIES LINKER_LANGUAGE CXX RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/$<0:>)
set_target_properties(stream PROPERTIES LINKER_LANGUAGE CXX RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/$<0:>)

