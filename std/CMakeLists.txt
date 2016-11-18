# lib std

add_lib_batch(ivm-std SHARED "*.c")

target_link_libraries(ivm-std pthread)

