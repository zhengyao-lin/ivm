import std
import ulist
import math
loc.merge(import graph)

loc inf = math.inf

loc g1 = Graph(7)

g1.dicon(0, 1, 7)
g1.dicon(0, 3, 5)
g1.dicon(1, 2, 8)
g1.dicon(1, 3, 9)
g1.dicon(1, 4, 7)
g1.dicon(2, 4, 5)
g1.dicon(3, 4, 15)
g1.dicon(3, 5, 6)
g1.dicon(4, 5, 8)
g1.dicon(4, 6, 9)
g1.dicon(5, 6, 11)

g1.prim(0).print()
g1.dijkstra(0).print()

loc g2 = Graph([
	[ inf, 2, 8, 1, inf, inf, inf, inf ],
	[ 2, inf, 6, inf, 1, inf, inf, inf ],
	[ 8, 6, inf, 7, 4, 2, 2, inf ],
	[ 1, 7, inf, inf, inf, inf, 9, inf ],
	[ inf, 1, 4, inf, inf, 3, inf, 9 ],
	[ inf, 2, inf, inf, 3, inf, 4, 6 ],
	[ inf, inf, 2, 9, inf, 4, inf, 2 ],
	[ inf, inf, inf, inf, 9, 6, 2, inf ]
], true)

g2.prim().print()
g2.dijkstra().print()

loc g3 = Graph(9)

g3.dicon(0, 1, 1)
g3.dicon(0, 2, 5) 
g3.dicon(1, 2, 3) 
g3.dicon(1, 3, 7) 
g3.dicon(1, 4, 5) 
g3.dicon(2, 4, 1) 
g3.dicon(2, 5, 7) 
g3.dicon(3, 4, 2) 
g3.dicon(3, 6, 3) 
g3.dicon(4, 5, 3)
g3.dicon(4, 6, 6)
g3.dicon(4, 7, 9) 
g3.dicon(5, 7, 5) 
g3.dicon(6, 7, 2) 
g3.dicon(6, 8, 7)
g3.dicon(7, 8, 4)

g3.dijkstra().print()

// -> "str: \\[ \\[ 0, 3, 5 \\], \\[ 3, 5, 6 \\], \\[ 0, 1, 7 \\], \\[ 1, 4, 7 \\], \\[ 4, 2, 5 \\], \\[ 4, 6, 9 \\] \\]"
// -> "str: \\[ \\[ 0, 3, 5, 6 \\], 22 \\]"
// -> "str: \\[ \\[ 0, 3, 1 \\], \\[ 0, 1, 2 \\], \\[ 1, 4, 1 \\], \\[ 4, 5, 3 \\], \\[ 4, 2, 4 \\], \\[ 2, 6, 2 \\], \\[ 6, 7, 2 \\] \\]"
// -> "str: \\[ \\[ 0, 1, 4, 2, 6, 7 \\], 11 \\]"
// -> "str: \\[ \\[ 0, 1, 2, 4, 3, 6, 7, 8 \\], 16 \\]"
