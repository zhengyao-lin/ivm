# lib vm

add_lib_batch(ivm-vm SHARED "*.c" "native/*.c" "mod/*.c" "thread/*.c")

# link libraries

if(WIN32)
	target_link_libraries(ivm-vm ivm-std m)
else()
	target_link_libraries(ivm-vm ivm-std m dl)
endif()
