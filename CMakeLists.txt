# haven't really tested, but the higher the better
cmake_minimum_required(VERSION 2.8)

PROJECT(ivm)

ADD_EXECUTABLE(test test.c)

# global variables
SET(IVM_PUB_HEAD_DIR ${PROJECT_SOURCE_DIR})
SET(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)

# set basic compile flags
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -g -Wpedantic -Wno-unused-value")

IF(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ENDIF()

IF(CMAKE_C_COMPILER MATCHES "clang")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu-label-as-value")
ENDIF()

# set flags for different versions
IF(VERSION STREQUAL "release")
	MESSAGE(STATUS "Compile in release mode")
	SET(CMAKE_BUILD_TYPE Release)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
ELSEIF((VERSION STREQUAL "debug") OR (NOT DEFINED VERSION))
	MESSAGE(STATUS "Compile in debug mode")
	SET(CMAKE_BUILD_TYPE Debug)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DIVM_DEBUG=1")
ELSE()
	MESSAGE(FATAL_ERROR "Unknown compile mode '${VERSION}'")
ENDIF()

# reset output dir
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# add include files
INCLUDE_DIRECTORIES(${IVM_PUB_HEAD_DIR})

# subdirs
ADD_SUBDIRECTORY(vm)
ADD_SUBDIRECTORY(std)
ADD_SUBDIRECTORY(mem)

# link libraries
TARGET_LINK_LIBRARIES(test vm mem)

MESSAGE(STATUS "Compile using argument: '${CMAKE_C_FLAGS}'")
