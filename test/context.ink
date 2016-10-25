a = fn: {
	ret fn: {
		print(the_msg) // -> "str: hey"
	}
}

a = a()

loc = {
	the_msg: "hey",
	a: a,
	print: print,
	typename: typename
}

a()

a = fn: {
	a = (fn: {
		ret fn: print(hello) // -> "str: hi"
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

print(what) // -> "str: yes"

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

// -> "str: loc add 10"
// -> "str: no ret"
// -> "str: everything is ok"
// -> "str: yo"
// -> "str: object"

// -> "num: 0"
// -> "num: 2"
// -> "str: none"
