# lib vm

add_lib_batch(vm SHARED "*.c" "gc/*.c")

# link libraries
target_link_libraries(vm std m)
