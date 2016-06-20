# lib util

ADD_LIBRARY(util STATIC
			parser.c
			env.c
			perf.c
			gen.c)

# reset output dir
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# add include files
INCLUDE_DIRECTORIES(${IVM_PUB_HEAD_DIR})

# link libraries
TARGET_LINK_LIBRARIES(util m)
