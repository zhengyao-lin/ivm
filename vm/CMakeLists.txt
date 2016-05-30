# lib vm

ADD_LIBRARY(vm SHARED
			obj.c
			num.c
			str.c
			slot.c
			vm.c
			exec.c
			instr.c
			context.c
			func.c
			call.c
			coro.c
			runtime.c
			op.c
			byte.c
			dbg.c)

# reset output dir
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# add include files
INCLUDE_DIRECTORIES(${IVM_PUB_HEAD_DIR})

# subdirs
ADD_SUBDIRECTORY(gc)
ADD_SUBDIRECTORY(std)

# dependencies
ADD_DEPENDENCIES(vm vm_gc vm_std)

# link libraries
TARGET_LINK_LIBRARIES(vm vm_gc vm_std)
