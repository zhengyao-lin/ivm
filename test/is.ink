print(none is none)
print(1 is numeric)
print((fn:0) is function)
print([] is list)
print("str" is string)
print(range(1) is range)

print(1 is 2)

loc class = fn p, init: {
	loc r = fn: {
		loc n = { proto: p }
		init(n)
		n
	}

	r.proto = p

	r
}

loc TypeA = class({
	val: "old",
	shout: fn: print(base.val)
}) { | self |
	self.val = "hi"
}

loc t = TypeA()

t.shout()

ret

// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "str: hi"
