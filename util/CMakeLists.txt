# lib util

file(GLOB c_list "*.c")
add_library(util SHARED ${c_list})

# link libraries
TARGET_LINK_LIBRARIES(util vm std m)
