# lib vm

ADD_LIBRARY(vm SHARED
			obj.c
			num.c
			str.c
			slot.c
			vm.c
			exec.c
			stack.c
			context.c
			func.c
			call.c
			coro.c
			runtime.c)

# reset output dir
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# add include files
INCLUDE_DIRECTORIES(${IVM_PUB_HEAD_DIR})

# subdirs
ADD_SUBDIRECTORY(gc)

# dependencies
ADD_DEPENDENCIES(vm vm_gc)

# link libraries
TARGET_LINK_LIBRARIES(vm vm_gc)
