# lib util

add_lib_batch(ivm-util SHARED "*.c")

# link libraries
TARGET_LINK_LIBRARIES(ivm-util ivm-vm ivm-mem ivm-std m)
