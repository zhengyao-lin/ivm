# lib vm

file(GLOB c_list "*.c" "gc/*.c")
add_library(vm SHARED ${c_list})

# link libraries
target_link_libraries(vm std m)
