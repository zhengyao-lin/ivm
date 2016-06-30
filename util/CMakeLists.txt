# lib util

add_lib_batch(util SHARED "*.c")

# link libraries
TARGET_LINK_LIBRARIES(util vm std m)
