# cmake version: haven't really tested, but the higher the better
# remove it if necessary
cmake_minimum_required(VERSION 2.8)
include(CMakeUtils.txt)

project(ivm)

add_executable(testbed test.c)

# global variables
set(IVM_PUB_HEAD_DIR ${PROJECT_SOURCE_DIR})
set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)

# set basic compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -g -Wpedantic -Wno-unused-value")

if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()

if(CMAKE_C_COMPILER MATCHES "clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu-label-as-value")
endif()

# set flags for different versions
if(VERSION STREQUAL "release")
	message(STATUS "Compile in release mode")
	set(CMAKE_BUILD_TYPE Release)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
elseif((VERSION STREQUAL "debug") OR (NOT DEFINED VERSION))
	message(STATUS "Compile in debug mode")
	set(CMAKE_BUILD_TYPE Debug)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DIVM_DEBUG=1")
else()
	message(FATAL_ERROR "Unknown compile mode '${VERSION}'")
endif()

# reset output dir
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# add include files
include_directories(${IVM_PUB_HEAD_DIR})

# subdirs
add_subdirectory(vm)
add_subdirectory(std)
add_subdirectory(mem)
add_subdirectory(util)

# link libraries
target_link_libraries(testbed vm mem util iasp)

message(STATUS "Compile using argument: '${CMAKE_C_FLAGS}'")

file(GLOB app_list "app/*")

foreach(dir ${app_list})
	add_subdirectory(${dir})
	message(STATUS "Find application ${dir}")
endforeach(dir ${app_list})

include(CMakeTests.txt)
