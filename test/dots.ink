import math
import ulist

loc pythago = fn a, b: math.sqrt(a * a + b * b)

loc Vec = fn x, y: {
	loc.+ = fn v2: Vec(x + v2.x, y + v2.y)
	loc.- = fn v2: Vec(x - v2.x, y - v2.y)
	loc.* = fn a: Vec(x * a, y * a)
	loc./ = fn a: Vec(x / a, y / a)

	loc.-@ = fn: Vec(-x, -y)
	loc.+@ = fn: clone loc

	loc.len = fn: pythago(x, y)
	loc.deg = fn: math.deg(math.atan2(y, x))
	loc.rad = fn: math.atan2(y, x)

	obj = loc

	loc.scalar_mul = fn v2: {
		x * v2.x + y * v2.y
	}

	loc.^ = fn v2: {
		if len() == 0: {
			ret math.atan2(y, x)
		}

		loc val = scalar_mul(v2) / (len() * v2.len())

		if val > 1: val = 1
		elif val < -1: val = -1

		math.acos(val)
	}

	loc.resolve = fn dv: { // direction vector
		loc theta = base ^ dv // included angle

		loc rv1 = dv * (len() * math.cos(theta) / dv.len())
		loc rv2 = base - rv1

		// print("here " + Vel(rv2).to_str())

		[rv1, rv2]
	}

	loc
}

loc Pos = Vec

loc Vel = fn mag, deg: {
	if typeof(mag) == "object": loc obj = mag
	else: {
		loc rad = math.rad(deg)
		loc mag_x = math.cos(rad) * mag
		loc mag_y = math.sin(rad) * mag

		loc obj = Vec(mag_x, mag_y)
	}

	obj.to_str = fn: {
		"vel: " + base.len() + "m/s, " + base.deg() + "deg"
	}

	obj.p = fn: {
		print(base.to_str())
	}

	obj
}

loc Dot = fn m, r, pos, vel, name: {
	loc p = fn prefix: {
		prefix = prefix || ""

		print(prefix + "-> dot '" + (base.name || "<no name>") + "'")
		print(prefix + "      m: " + base.m)
		print(prefix + "      r: " + base.r)
		print(prefix + "      x: " + base.pos.x + ", y: " + base.pos.y)
		print(prefix + "      v: " + base.vel.to_str())
	}

	loc.next = []

	loc
}

loc Arena = fn h, w: {
	loc tryCollide = fn d1, d2: {
		loc col_v = d2.pos - d1.pos

		if col_v.len() > d1.r + d2.r: ret 0

		// print("### collision: '" + (d1.name || "<no name>") + "' and '" + (d2.name || "<no name>") + "'")
		
		[ loc.v1_sub1, loc.v1_sub2 ] = d1.vel.resolve(col_v)
		[ loc.v2_sub1, loc.v2_sub2 ] = d2.vel.resolve(col_v)
		// velocity resolve
		
		// print("col_v: " + Vel(col_v).to_str())
		// print("resolved v1: " + Vel(v1_sub1).to_str() + ", " + Vel(v1_sub2).to_str())
		// print("resolved v2: " + Vel(v2_sub1).to_str() + ", " + Vel(v2_sub2).to_str())

		loc v1 = v1_sub1.len()
		loc v2 = -v2_sub1.len()
		// collinear collision
		
		// print(v1 + ", " + v2)

		loc m1 = d1.m
		loc m2 = d2.m

		loc v1n = (2 * m2 * v2 + (m1 - m2) * v1) / (m1 + m2)
		loc v2n = (2 * m1 * v1 + (m2 - m1) * v2) / (m1 + m2)
		// formulas

		// print(v1n + ", " + v2n)

		if v1: {
			v1_sub1 = v1_sub1 * (v1n / v1)
		} else: {
			v1_sub1 = Vel(v1n, col_v.deg())
		}
		//d1.vel = Vel(v1_sub1 + v1_sub2)
		d1.next.push(v1_sub1 + v1_sub2)

		if v2: {
			v2_sub1 = v2_sub1 * (v2n / v2)
		} else: {
			v2_sub1 = Vel(v2n, col_v.deg())
		}
		// d2.vel = Vel(v2_sub1 + v2_sub2)
		d2.next.push(v2_sub1 + v2_sub2)

		// print("final vel1: " + Vel(v1_sub1 + v1_sub2).to_str())
		// print("final vel2: " + Vel(v2_sub1 + v2_sub2).to_str())

		// d1.period = period
		// d2.period = period
		// update velocity and period
	
		1
	}

	{
		height: h,
		width: w,

		period: 0,

		dots: [],

		changePos: fn span: {
			for loc dot in base.dots:
				dot.pos = dot.pos + dot.vel * span
		},

		checkCol: fn: {
			loc i = 0
			loc dots = base.dots
			loc size = dots.size()

			while i < size: {
				loc j = i + 1
				while j < size: {
					tryCollide(dots[i], dots[j], period)
					j = j + 1
				}
				i = i + 1
			}

			for loc dot in dots: {
				if dot.next.size(): {
					dot.vel = Vel(dot.next.sum())
					dot.next = []
				}
			}
		},

		calc: fn span: {
			span = span || 0.04 // sec
			base.period = base.period + 1

			base.changePos(span)
			base.checkCol()
		},

		addDot: fn d: {
			base.dots.push(d)
		},

		state: fn prefix: {
			for loc dot in base.dots:
				dot.p(prefix)
		}
	}
}

loc v1 = Vel(4, 30)
loc v2 = Vel(3, 120)

v1.p()
v2.p()

Vel(v1 + v2).p()
print(math.deg(v1 ^ v2))

loc col_v = Vel(4, -30)
loc mov_v = Vel(5, 30)

[rv1, rv2] = mov_v.resolve(col_v)

Vel(rv1).p()
Vel(rv2).p()

print("################ sim!! ################")

loc test1 = fn: {
	world = Arena(10, 10)

	d1 = Dot(1, 0.05, Pos(1, 1), Vel(1, 45), "a")
	d2 = Dot(1, 0.05, Pos(4, 4), Vel(2, 45 + 180), "b")

	world.addDot(d1)
	world.addDot(d2)

	i = 0

	print("### initial state ###")
	world.state("   ")
	print("")

	while i < 100: {
		world.calc()
		i = i + 1
	}

	print("")
	print("### after 100 period ###")
	world.state("   ")
}

loc test2 = fn: {
	world = Arena(10, 10)

	d1 = Dot(1, 0.05, Pos(1, 1), Vel(1, 45), "a")
	d2 = Dot(10000, 0.05, Pos(4, 4), Vel(0, 0), "b")

	world.addDot(d1)
	world.addDot(d2)

	i = 0

	print("### initial state ###")
	world.state("   ")
	print("")

	while i < 104: {
		world.calc()
		i = i + 1
	}

	print("")
	print("### after 100 period ###")
	world.state("   ")
}

loc test3 = fn: {
	world = Arena(10, 10)

	d1 = Dot(1, 1, Pos(1, 1), Vel(1, 45), "a")
	d2 = Dot(1, 1, Pos(3, 2), Vel(0, 0), "b")

	world.addDot(d1)
	world.addDot(d2)

	i = 0

	print("### initial state ###")
	world.state("   ")
	print("")

	while i < 100: {
		world.calc()
		i = i + 1
	}

	print("")
	print("### after 100 period ###")
	world.state("   ")
}

loc test4 = fn: {
	world = Arena(10, 10)

	d1 = Dot(1, 1, Pos(5, 3), Vel(0, 0), "a")
	d2 = Dot(1, 1, Pos(5, 5), Vel(0, 0), "b")
	d3 = Dot(1, 1, Pos(0, 4), Vel(1, 0), "c")

	world.addDot(d1)
	world.addDot(d2)
	world.addDot(d3)

	i = 0

	print("### initial state ###")
	world.state("   ")
	print("")

	while i < 100: {
		world.calc()
		i = i + 1
	}

	print("")
	print("### after 100 period ###")
	world.state("   ")
}

loc test5 = fn: {
	world = Arena(10, 10)

	d1 = Dot(1, 1, Pos(0, 0), Vel(0.1, 45), "a")
	d2 = Dot(1, 1, Pos(0, 2), Vel(0.1, -45), "b")
	d3 = Dot(1, 1, Pos(2, 0), Vel(0.1, 180 - 45), "c")
	d4 = Dot(1, 1, Pos(2, 2), Vel(0.1, 180 + 45), "d")

	world.addDot(d1)
	world.addDot(d2)
	world.addDot(d3)
	world.addDot(d4)

	i = 0

	print("### initial state ###")
	world.state("   ")
	print("")

	while i < 1000: {
		world.calc()
		i = i + 1
	}

	print("")
	print("### after 100 period ###")
	world.state("   ")
}

test1()
test2()
test3()
test4()
test5()

ret

// -> "str: vel: 4m/s, 30deg"
// -> "str: vel: 2.9999999999999996m/s, 120deg"
// -> "str: vel: 5m/s, 66.86989764584402deg"
// -> "num: 90"
// -> "str: vel: 2.5000000000000018m/s, -30deg"
// -> "str: vel: 4.330127018922193m/s, 60.000000000000014deg"
// -> "str: ################ sim!! ################"
// -> "str: ### initial state ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 0.05"
// -> "str:          x: 1, y: 1"
// -> "str:          v: vel: 1m/s, 45deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 1"
// -> "str:          r: 0.05"
// -> "str:          x: 4, y: 4"
// -> "str:          v: vel: 2m/s, -135.00000000000003deg"
// -> "str: "
// -> "str: "
// -> "str: ### after 100 period ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 0.05"
// -> "str:          x: -1.6870057685088853, y: -1.6870057685088853"
// -> "str:          v: vel: 2.0000000000000004m/s, -135deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 1"
// -> "str:          r: 0.05"
// -> "str:          x: 3.8585786437627054, y: 3.8585786437627054"
// -> "str:          v: vel: 1m/s, 45.00000000000002deg"
// -> "str: ### initial state ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 0.05"
// -> "str:          x: 1, y: 1"
// -> "str:          v: vel: 1m/s, 45deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 10000"
// -> "str:          r: 0.05"
// -> "str:          x: 4, y: 4"
// -> "str:          v: vel: 0m/s, 0deg"
// -> "str: "
// -> "str: "
// -> "str: ### after 100 period ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 0.05"
// -> "str:          x: 3.9415642097360424, y: 3.9415642097360424"
// -> "str:          v: vel: 0.9998000199979997m/s, -135deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 10000"
// -> "str:          r: 0.05"
// -> "str:          x: 4, y: 4"
// -> "str:          v: vel: 1.9998000199979998e-4m/s, 44.99999999999999deg"
// -> "str: ### initial state ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 1, y: 1"
// -> "str:          v: vel: 1m/s, 45deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 3, y: 2"
// -> "str:          v: vel: 0m/s, 0deg"
// -> "str: "
// -> "str: "
// -> "str: ### after 100 period ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 0.6557309122540738, y: 2.4163737689506175"
// -> "str:          v: vel: 0.3584961458910427m/s, 113.99213211527479deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 6.172696212492113, y: 3.4120533557955834"
// -> "str:          v: vel: 0.9335312064314016m/s, 23.99213211527478deg"
// -> "str: ### initial state ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 5, y: 3"
// -> "str:          v: vel: 0m/s, 0deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 5, y: 5"
// -> "str:          v: vel: 0m/s, 0deg"
// -> "str:    -> dot 'c'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 0, y: 4"
// -> "str:          v: vel: 1m/s, 0deg"
// -> "str: "
// -> "str: "
// -> "str: ### after 100 period ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 5.538108326596612, y: 2.6871463217461633"
// -> "str:          v: vel: 0.8645071866841925m/s, -30.173520029644358deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 5.538108326596612, y: 5.312853678253845"
// -> "str:          v: vel: 0.8645071866841925m/s, 30.173520029644358deg"
// -> "str:    -> dot 'c'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 3.643783346806795, y: 4"
// -> "str:          v: vel: 0.5052546483427656m/s, 0deg"
// -> "str: ### initial state ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 0, y: 0"
// -> "str:          v: vel: 0.1m/s, 44.99999999999999deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 0, y: 2"
// -> "str:          v: vel: 0.1m/s, -44.99999999999999deg"
// -> "str:    -> dot 'c'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 2, y: 0"
// -> "str:          v: vel: 0.1m/s, 135deg"
// -> "str:    -> dot 'd'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 2, y: 2"
// -> "str:          v: vel: 0.1m/s, -135deg"
// -> "str: "
// -> "str: "
// -> "str: ### after 100 period ###"
// -> "str:    -> dot 'a'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 0.002828427124746191, y: 0.00282842712474619"
// -> "str:          v: vel: 0m/s, 0deg"
// -> "str:    -> dot 'b'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 0.002828427124746191, y: 1.9971715728752537"
// -> "str:          v: vel: 0m/s, 1.6792996808955434e-5deg"
// -> "str:    -> dot 'c'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 1.9971715728752537, y: 0.002828427124746191"
// -> "str:          v: vel: 0m/s, -153.90673266977876deg"
// -> "str:    -> dot 'd'"
// -> "str:          m: 1"
// -> "str:          r: 1"
// -> "str:          x: 1.9971715728752537, y: 1.9971715728752537"
// -> "str:          v: vel: 0m/s, -4.553457607660175e-6deg"
