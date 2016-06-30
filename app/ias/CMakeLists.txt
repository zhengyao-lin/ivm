# ias

add_exe_batch(ias "*.c")
add_lib_batch(iasp SHARED "*.c")

# link libraries
target_link_libraries(ias vm mem util)
target_link_libraries(iasp vm mem util)
