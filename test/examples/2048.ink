import std
import core2048

/*
loc p = CorePlate(3, 3, [
	[ 2, 2, 2, 2 ],
	[ 2, 0, 2, 0 ],
	[ 2, 2, 0, 0 ],
	[ 0, 2, 2, 4 ],
	[ 0, 0, 0, 0 ]
])

p.slide(1)
p.slide(2)
p.slide(2)
*/

loc p = core2048.CorePlate(4, 4)

p.print()
p.start()

ret
