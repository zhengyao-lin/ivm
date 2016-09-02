# ilang

add_exe_batch(ilang "*.c" "gen/*.c")

# link libraries
target_link_libraries(ilang ivm-vm ivm-util)
