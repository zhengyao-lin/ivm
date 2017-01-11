# cmake version: haven't really tested, but the higher the better
# remove it if necessary
cmake_minimum_required(VERSION 3.0)
include(CMakeUtils.txt)

project(ivm)

##################################### compiler settings #####################################

# global variables
set(IVM_PUB_HEAD_DIR ${PROJECT_SOURCE_DIR})
set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)

# cross compilation

file(GLOB plat_list "platform/*")
foreach(dir ${plat_list})
	if(dir MATCHES "(.*)(/|\\\\)CMakePlatform(.*)\\.txt")
		message(STATUS "Find platform ${CMAKE_MATCH_3}")
		set(PLATFORMS "${CMAKE_MATCH_3}" ${PLATFORMS})
	endif()
endforeach(dir ${plat_list})

# message(STATUS "${PLATFORMS}")

if(DEFINED PLATFORM)
	list(FIND PLATFORMS "${PLATFORM}" idx)
	# message(STATUS "${idx}")

	if(NOT idx EQUAL -1)
		message(STATUS "Target platform ${PLATFORM}")
		include(platform/CMakePlatform${PLATFORM}.txt)
	else()
		message(FATAL_ERROR "Unrecognized PLATFORM ${PLATFORM}")
	endif()
endif()

# set basic compile flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic -Wno-unused-function")

# struct timespec
add_definitions(-D__need_timespec)

if(NOT CMAKE_C_COMPILER MATCHES "icc")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-value")
endif()

if(CMAKE_C_COMPILER MATCHES "clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu-label-as-value")
endif()

if(CMAKE_C_COMPILER MATCHES "gcc" AND NOT CMAKE_C_COMPILER MATCHES "arm")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-ms-bitfields")
endif()

# avoid warnings on MinGW
if(NOT WIN32)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
	# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIVM_LIB_PATH=\\\"${CMAKE_INSTALL_PREFIX}/lib\\\"")
else()
	# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIVM_LIB_PATH=\\\"${CMAKE_INSTALL_PREFIX}\\\"")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIVM_DLL_SUFFIX=\\\"${CMAKE_SHARED_LIBRARY_SUFFIX}\\\"")

# set flags for different versions
if(VERSION STREQUAL "release")
	
	# real releasing mode
	message(STATUS "Compile in release mode")
	set(CMAKE_BUILD_TYPE Release)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

elseif(VERSION STREQUAL "prerelease")
	
	# prereleasing mode
	message(STATUS "Compile in prerelease mode")
	# set(CMAKE_BUILD_TYPE Release)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -DIVM_DEBUG=1")

elseif((VERSION STREQUAL "debug") OR (NOT DEFINED VERSION))

	# normal debug mode
	message(STATUS "Compile in debug mode")
	set(CMAKE_BUILD_TYPE Debug)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DIVM_DEBUG=1")

else()
	message(FATAL_ERROR "Unknown compile mode '${VERSION}'")
endif()

# other flags

if((MULTITHREAD STREQUAL "enabled") OR (NOT DEFINED MULTITHREAD))
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIVM_USE_MULTITHREAD=1")
	set(USE_PTHREAD 1)
	message(STATUS "Thread enabled")
elseif(MULTITHREAD STREQUAL "disabled")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIVM_USE_MULTITHREAD=0")
	set(USE_PTHREAD 0)
	message(STATUS "Thread disabled")
endif()

# reset output dir
if(WIN32)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
	set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
else()
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
	set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
endif()

if(NOT DEFINED LIB_PATH)
	set(LIB_PATH ${LIBRARY_OUTPUT_PATH})
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIVM_LIB_PATH=\\\"${LIB_PATH}\\\"")

# add include files
include_directories(${IVM_PUB_HEAD_DIR})

##################################### compiler settings #####################################

# subdirs
add_subdirectory(vm)
add_subdirectory(std)
add_subdirectory(util)

# search for mods
file(GLOB mod_list "mod/*")
foreach(dir ${mod_list})
	add_subdirectory(${dir})
	message(STATUS "Find module ${dir}")
endforeach(dir ${mod_list})

file(MAKE_DIRECTORY "${LIBRARY_OUTPUT_PATH}/ivm/mod")

# search for apps
file(GLOB app_list "app/*")
foreach(dir ${app_list})
	add_subdirectory(${dir})
	message(STATUS "Find application ${dir}")
endforeach(dir ${app_list})

# test exe
add_executable(testbed test.c)
target_link_libraries(testbed ivm-vm ivm-util)

# add_library(testmod SHARED testmod.c)
# target_link_libraries(testmod ivm-vm)

if(DEFINED MEMCHECK AND (MEMCHECK STREQUAL "yes"))
	message(STATUS "Memory check enabled")
	find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
	set(CMAKE_MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full")
else()
	message(STATUS "Memory check disabled")
endif()

include(CMakeTests.txt)

message(STATUS "Compile using flags: '${CMAKE_C_FLAGS}'")
