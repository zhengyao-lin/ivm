import std
import ulist
import curses
import core2048
import time

try: (fn: {
	loc.merge(curses)

	initscr()
	cbreak()
	noecho()
	intrflush(false)
	stdscr.keypad(true)

	loc m = 4 // row
	loc n = 4 // column
	loc win = []
	loc plate = core2048.CorePlate(m, n)
	loc center_msg = none

	loc draw_win = fn: {
		[ line, col ] = stdscr.size()

		clear()

		if center_msg != none: {
			loc pos = (col - center_msg.len()) / 2
			loc pos = pos < 0 ? 0 : pos
			stdscr.move(0, pos)
			stdscr.addstr(center_msg)
		}

		for loc w in win: {
			w.delbg(attr.reverse)
			w.clear()
			w.remove()
		}
		
		win = []

		// stdscr.refresh()

		loc vmargin = 4 // in total
		loc hmargin = 6

		loc vgap = 1 // the gap size between two vertical blocks
		loc hgap = 2

		loc st_l = (vmargin / 2).round()
		loc st_c = (hmargin / 2).round()

		loc height = ((line - vmargin - (vgap * (n - 1))) / n).round()
		loc width = ((col - hmargin - (hgap * (m - 1))) / m).round()

		if height % 2:
			loc mid_line = (height - 1) / 2
		else:
			loc mid_line = height / 2

		if height <= 0 || width <= 0:
			ret

		// print([ height, width ])

		loc cur_l = st_l
		
		for i in range(m): {
			loc cur_c = st_c
			
			for j in range(n): {
				loc w = window(height, width, cur_l, cur_c)
				win.push(w)
				w.addbg(attr.reverse | attr.bold)

				loc s = plate.get(i, j).to_s()
				loc pos = (width - s.len()) / 2
				loc pos = pos < 0 ? 0 : pos

				w.move(mid_line, pos)

				w.addstr(s)

				cur_c += width + hgap
			}

			cur_l += height + vgap
		}

		stdscr.refresh()
	}

	while 1: {
		clear()
		draw_win()
		
		loc c = none
		loc dir = none

		// stdscr.clear()
		while 1: {
			dir = none
			c = getch()

			if c == key.resize: dir = -2
			elif c == key.up: dir = 0
			elif c == key.down: dir = 1
			elif c == key.left: dir = 2
			elif c == key.right: dir = 3
			elif c == "q".ord(): dir = -1

			if dir != none: {
				break
			}

			stdscr.move(0, 0)
			stdscr.addstr("unknown direction(press q to exit)")
		}

		// print(dir)

		if dir == -2:
			cont

		if dir == -1:
			break

		if !plate.next_turn(dir): {
			loc center_msg = "good game! (press q to exit)"
			draw_win()

			for i in range(10): {
				if i % 2:
					stdscr.addbg(attr.reverse)
				else:
					stdscr.delbg(attr.reverse)
				stdscr.refresh()
				time.msleep(200)
			}
			stdscr.delbg(attr.reverse)
			draw_win()
			
			while 1: {
				loc c = getch()
				if c == key.resize: draw_win()
				elif c == "q".ord(): break
			}

			break
		}
	}

})() catch top e: none final: curses.endwin()

if e != none:
	print(e)

ret
