import std

loc b = 1

a = fn: {
	loc = b
	b.val = 10
	print(val)
}

a()

a = fn: {
	loc.merge({ val: 1 })
	print(val)
}

a()

a = fn: {
	ret fn: {
		print(the_msg)
	}
}

a = a()

loc = {
	the_msg: "hey",
	a: a,
	print: print,
	string: string,
	numeric: numeric,
	typename: typename
}

a()

a = fn: {
	a = (fn: {
		ret fn: print(hello)
	})()
	hello = "hi"
	a()
}

a()

d1 = 10
d2 = 123
d3 = 1024

local = loc
a = local.clone()

local.what = "yes"

print(what)

loc.+ = fn b: {
	print("loc add " + b)
	"no ret"
}

print(loc + 10)

loc a = fn: {
	loc = top
	top = {}
	top = loc
}

a()

print("everything is ok")

loc link = fn ra, rb: {
	deref ra = loc
	deref rb = loc
	none
}

link(ref loc.a, ref loc.b)

a.bala = "yo"

print(b.bala)

a.proto = "no!!"

print(typename(b.proto))

(fn: {
	loc a = { val: 0 }
	a.a = a
	loc = a
	print(val)
	a.val = 2
	print(val)

	del loc

	print(typename(val))
})()

// -> "num: 10"
// -> "num: 1"

// -> "str: hey"
// -> "str: hi"
// -> "str: yes"

// -> "str: loc add 10"
// -> "str: no ret"
// -> "str: everything is ok"
// -> "str: yo"
// -> "str: object"

// -> "num: 0"
// -> "num: 2"
// -> "str: none"
