# curses

find_library(NCURSES_LIB ncurses)

if(NOT NCURSES_LIB_NOTFOUND)
	add_mod_batch(curses main.c)
	target_link_libraries(curses ivm-vm ncurses)
endif()
