# lib std

file(GLOB c_list "*.c")
add_library(std SHARED ${c_list})
