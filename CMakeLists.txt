# haven't really tested, but the higher the better
cmake_minimum_required(VERSION 2.8)

PROJECT(ivm)

ADD_EXECUTABLE(test test.c)

# global variables
SET(IVM_PUB_HEAD_DIR ${PROJECT_SOURCE_DIR})
SET(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)

# set compiler flags
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -pedantic -Wno-variadic-macros")

# reset output dir
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# add include files
INCLUDE_DIRECTORIES($(IVM_PUB_HEAD_DIR))

# subdirs
ADD_SUBDIRECTORY(vm)
ADD_SUBDIRECTORY(mem)

# link libraries
TARGET_LINK_LIBRARIES(test vm mem)
