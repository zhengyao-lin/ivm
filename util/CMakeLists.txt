# lib util

file(GLOB c_list "*.c")
add_library(util STATIC ${c_list})

# link libraries
TARGET_LINK_LIBRARIES(util m)
