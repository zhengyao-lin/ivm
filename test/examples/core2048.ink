import std
import math
import ulist
import io

loc Vector = fn i, j: {
	i: i, j: j,
	
	+: fn v: Vector(base.i + v.i, base.j + v.j),
	+=: fn v: Vector(base.i + v.i, base.j + v.j),
	-: fn v: Vector(base.i - v.i, base.j - v.j),
	-=: fn v: Vector(base.i - v.i, base.j - v.j),
	-@: fn: Vector(-base.i, -base.j),
	==: fn v: v.i == base.i && v.j == base.j,
	rev: fn: Vector(base.j, base.i),

	to_s: fn: "(" + base.i + ", " + base.j + ")"
}

// n rows, m columns
loc CorePlate = fn
	n = 4, m = 4,
	plate = {
		[ [ 0 for loc j in range(m) ] for loc i in range(n) ]
	},
	rd = [ 2, 4 ] /* random init number */,
	rd_poss = [ 0.8, 0.2 ],
	init_n = 2: {

	plate.to_s = fn: {
		[ loc n, loc m ] = base.mat()
		loc r = "\n"

		for loc i in range(n): {
			r += "   "
			for loc j in range(m): {
				r += loc tmp = base[i][j].to_s()
				if j != (m - 1): {
					r += " " * (5 - tmp.len())
				}
			}
			if i != (n - 1): r += "\n"
		}

		r
	}

	loc c_lu = Vector(0, 0) // left up
	loc c_ld = Vector(n - 1, 0) // left down
	loc c_ru = Vector(0, m - 1) // right up
	loc c_rd = Vector(n - 1, m - 1) // right down

	loc d_u = Vector(-1, 0)
	loc d_d = Vector(1, 0)
	loc d_l = Vector(0, -1)
	loc d_r = Vector(0, 1)

	loc get = fn p: plate[p.i][p.j]
	loc set = fn p, val: plate[p.i][p.j] = val

	// whether the point exceed the plate
	loc inplate_c = fn i, j: i >= 0 && i < n && j >= 0 && j < m
	loc inplate = fn p: p.i >= 0 && p.i < n && p.j >= 0 && p.j < m

	// print(plate)
	// print([ c_lu, c_ld, c_ru, c_rd ])
	// 
	
	loc all_pos = []

	for loc i in range(n):
		for loc j in range(m):
			all_pos.push(Vector(i, j))

	loc i = 0
	loc poss = init_n / (m * n)
	loc cnt = 0

	for loc i in range(n):
		for loc j in range(m): {
			if math.random() < poss && cnt < init_n: {
				cnt += 1
				plate[i][j] = rd.choose(rd_poss)
			}
		}

	if cnt < init_n: {
		for i in range(init_n - cnt):
			set(all_pos.choose(), rd.choose(rd_poss))
	}

	// toss a 2/4 on the empty pos, return whether the plate is full
	loc toss = fn: {
		loc empty = [ p for loc p in all_pos if get(p) == 0 ]
		if empty.size() == 0:
			false
		else: {
			set(empty.choose(), rd.choose(rd_poss))
			empty.size() > 1
		}
	}

	// if the game ends
	loc check_end = fn: {
		for loc i in range(n):
			for loc j in range(m): {
				if plate[i][j] == 0: ret false

				if inplate_c(i, j + 1) &&
				   plate[i][j] != 0 &&
				   plate[i][j] == plate[i][j + 1]:
					ret false

				if inplate_c(i + 1, j) &&
				   plate[i][j] != 0 &&
				   plate[i][j] == plate[i + 1][j]:
					ret false
			}

		ret true
	}

	{
		// dir:
		//   0. up
		//   1. down
		//   2. left
		//   3. right
		slide: fn dir: {
			print([ "up", "down", "left", "right" ][dir])

			loc corner = [
				c_ru, c_ld, c_ld, c_ru
			][dir] // start trav from this point

			loc base_len = [
				m, m, n, n
			][dir]

			loc edge_len = [
				n, n, m, m
			][dir]

			loc dir = [
				d_u, d_d, d_l, d_r
			][dir]

			loc rot90 = dir.rev() // u -> l
			loc rdir = -dir

			loc bas = corner

			loc move_merge = fn p: {
				loc init = p
				loc cur_val = get(p)
				loc r = true

				if cur_val == 0: ret false

				// print("cur " + cur_val)

				set(p, 0) // clear current point

				while inplate(p += dir): {
					loc v = get(p)
					// print("hey " + p.to_s())
					if v == cur_val: {
						// print("same! " + cur_val)
						// print("final " + p.to_s())
						set(p - dir, -1) // set barrier
						set(p, cur_val * 2)
						ret true
					}

					if v == -1: { // barrier
						set(p, cur_val)
						ret true
					}

					if v != 0: break
				}

				loc fin = p - dir
				if fin == init:
					r = false // no move

				set(fin, cur_val)
				
				ret r
			}

			loc clear_barrier = fn: {
				for i in range(n):
					for j in range(m):
						if plate[i][j] == -1:
							plate[i][j] = 0
			}

			loc moved = false

			for loc i in range(base_len): {
				loc cur = bas
				for loc j in range(edge_len): {
					// print(cur)
					moved |= move_merge(cur)
					// print(plate)
					cur += rdir // reverse scan
				}
				bas += rot90 // move base
			}

			clear_barrier()

			ret moved

			// print(plate)
		},

		start: fn: {
			loc dir_map = {
				w: 0, // up
				s: 1, // down
				a: 2, // left
				d: 3,  // right
				stop: -1
			}

			while 1: {
				loc act = input("a direction or 'stop' to exit: ")
				loc dir = dir_map[act]
				if dir is numeric: {
					if dir == -1:
						break
					else: {
						if base.slide(dir): {
							if !toss(): {
								if check_end(): {
									io.stdout.write("good game\n")
									base.print()
									break
								}
							}
						}
						base.print()
					}
				} else: {
					io.stdout.write("unknown direction\n")
				}
			}
		},

		print: fn: {
			print(plate)
		}
	}
}

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

loc p = CorePlate(4, 4)

p.print()
p.start()

ret
