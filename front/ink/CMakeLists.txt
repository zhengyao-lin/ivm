# ink

add_exe_batch(ink "*.c" "gen/*.c")

# link libraries
target_link_libraries(ink ivm-vm ivm-util)
