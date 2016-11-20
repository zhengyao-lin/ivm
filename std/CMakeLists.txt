# lib std

add_lib_batch(ivm-std SHARED "*.c")

if(USE_PTHREAD)
	target_link_libraries(ivm-std pthread m)
else()
	target_link_libraries(ivm-std m)
endif()
