# lib std

add_lib_batch(ivm-std SHARED "*.c")

if(USE_PTHREAD)
	find_package(Threads)
	if(CMAKE_USE_PTHREADS_INIT)
		message(STATUS "Found pthread")
		target_link_libraries(ivm-std pthread m)
	else()
		message(ERROR "Could not find pthread(you may set -DMULTITHREAD=disabled to disable thread)")
	endif()
else()
	target_link_libraries(ivm-std m)
endif()
