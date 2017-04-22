# ias

add_exe_batch(ias "*.c")

# link libraries
target_link_libraries(ias ivm-vm ivm-util)
