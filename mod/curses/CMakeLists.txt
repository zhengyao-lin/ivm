# curses

if (IVM_CROSS_COMPILE)
	# TODO: fix this
	add_mod_batch(curses main.c)
	target_link_libraries(curses ivm-vm curses)
else()
	find_package(Curses)

	if(CURSES_FOUND)
		message(STATUS "Found curses")

		if(CURSES_HAVE_CURSES_H)
			add_definitions(-DUSE_CURSES_H)
			include_directories(${CURSES_INCLUDE_DIR})
		elseif(CURSES_HAVE_NCURSES_H)
			add_definitions(-DUSE_NCURSES_H)
			include_directories(${CURSES_INCLUDE_DIR})
		elseif(CURSES_HAVE_NCURSES_NCURSES_H)
			add_definitions(-DUSE_NCURSES_H)
			include_directories(${CURSES_INCLUDE_DIR}/ncurses)
		elseif(CURSES_HAVE_NCURSES_CURSES_H)
			add_definitions(-DUSE_CURSES_H)
			include_directories(${CURSES_INCLUDE_DIR}/ncurses)
		endif()

		add_mod_batch(curses main.c)
		target_link_libraries(curses ivm-vm ${CURSES_LIBRARIES})
	else()
		message(STATUS "Could not find curses - curses module will not be compiled")
	endif()
endif()
