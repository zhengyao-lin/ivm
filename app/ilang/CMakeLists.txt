# ilang

add_exe_batch(ilang "*.c")

# link libraries
target_link_libraries(ilang vm mem util)
