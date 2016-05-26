# lib vm.std

ADD_LIBRARY(vm_std STATIC
			list.c
			chain.c
			pool.c
			hash.c)

# reset output dir
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# add include files
INCLUDE_DIRECTORIES(${IVM_PUB_HEAD_DIR})
INCLUDE_DIRECTORIES(${IVM_PUB_HEAD_DIR}/vm)
