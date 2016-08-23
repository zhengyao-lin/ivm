# lib vm

add_lib_batch(ivm-vm SHARED "*.c" "gc/*.c" "native/*.c" "mod/*.c")

# link libraries
target_link_libraries(ivm-vm ivm-std m dl)
