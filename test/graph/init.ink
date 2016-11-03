import math
$import("../ulist")
loc inf = math.inf

loc Graph = fn arg, is_graph = false: {
	if is_graph: {
		loc ws = arg
		loc n = arg.size()
	} else: {
		loc n = arg
		loc ws = [ [ (if i == j: 0 else: inf) for loc j in range(n) ] for loc i in range(n) ]
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
				loc min_w = inf
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
			loc prev = [ (if ws[a][i] != inf: 0 else: none) for loc i in range(n) ]

			while true: {
				// dist.print()

				loc min_d = inf
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
		},

		max_flow: fn s = 0, t = n - 1: {
			loc flow = [ [ (if j == inf: 0 else: j) for loc j in i ] for loc i in ws ]

			// flow.print()
			// ret

			loc find_aug = fn from, min_c, path = [], reached = [ false ] * n: {
				path.push(from)
				reached[from] = true
				// print(from)

				if from == t: ret [ path, min_c ]

				for loc next in range(n): {
					loc fl = flow[from][next]

					if fl && !reached[next]: {
						loc r = find_aug(next, (fl < min_c && fl || min_c), path, reached)
						if r: {
							ret r
						}
					}
				}

				path.pop()
				reached[from] = false

				ret none
			}

			loc max = 0

			while (loc aug = find_aug(s, inf)) != none: {
				[ loc path, loc min_fl ] = aug
				max = max + min_fl

				// aug.print()

				for loc p in range(path.size() - 1): {
					loc i = path[p]
					loc j = path[p + 1]
					flow[i][j] = flow[i][j] - min_fl
					flow[j][i] = flow[j][i] + min_fl
				}
			}

			// flow.print()
			// print(max)

			ret max
		}
	}
}
