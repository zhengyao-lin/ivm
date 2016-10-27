import ulist
import math

loc Graph = fn arg, is_graph = false: {
	if is_graph: {
		loc ws = arg
		loc n = arg.size()
	} else: {
		loc n = arg
		loc ws = [ [ (if i == j: 0 else: math.inf) for loc j in range(n) ] for loc i in range(n) ]
	}

	ret {
		con: fn a, b, w = 0: {
			ws[a][b] = w
		},

		dicon: fn a, b, w = 0: {
			ws[a][b] = ws[b][a] = w
		},

		weight: fn a, b: {
			ws[a][b]
		},

		prim: fn init = 0 /* initial point */: {
			loc outs = [ init ]
			loc rest = [ i for loc i in range(n) ]
			loc path = []

			rest[init] = none

			while true: {
				loc min_w = math.inf
				loc min_pf = none // from
				loc min_p = none

				for loc o in outs:
					for loc p in rest:
						if p != none && ws[o][p] < min_w: {
							min_w = ws[o][p]
							min_pf = o
							min_p = p
						}

				if min_p == none: break

				outs.push(min_p)
				rest[min_p] = none
				path.push([ min_pf, min_p, min_w ])
			}

			path
		},

		dijkstra: fn a = 0, b = n - 1: {
			loc outs = [ a ]
			loc dist = ws[a].clone()
			loc prev = [ (if ws[a][i] != math.inf: 0 else: none) for loc i in range(n) ]

			while true: {
				// dist.print()

				loc min_d = math.inf
				loc min_p = none

				for loc p in range(n): {
					if !outs.has(p) && dist[p] < min_d: {
						min_d = dist[p]
						min_p = p
					}
				}

				if min_p == none: break

				for loc p in range(n): {
					loc tmp = min_d + ws[min_p][p]
					if tmp < dist[p]: {
						dist[p] = tmp
						prev[p] = min_p
					}
				}

				outs.push(min_p)
			}

			// prev.print()

			loc cur = b
			loc path = [ cur ]

			while cur != a: {
				cur = prev[cur]
				path.push(cur)
			}

			ret [ path.reverse(), dist[b] ]
		}
	}
}

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
	[math.inf, 2, 8, 1, math.inf, math.inf, math.inf, math.inf],
	[2, math.inf, 6, math.inf, 1, math.inf, math.inf, math.inf],
	[8, 6, math.inf, 7, 4, 2, 2, math.inf],
	[1, 7, math.inf, math.inf, math.inf, math.inf, 9, math.inf],
	[math.inf, 1, 4, math.inf, math.inf, 3, math.inf, 9],
	[math.inf, 2, math.inf, math.inf, 3, math.inf, 4, 6],
	[math.inf, math.inf, 2, 9, math.inf, 4, math.inf, 2],
	[math.inf, math.inf, math.inf, math.inf, 9, 6, 2, math.inf]
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
