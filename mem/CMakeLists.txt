# lib mem

file(GLOB c_list "*.c")
add_library(mem SHARED ${c_list})
