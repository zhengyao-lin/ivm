print(none is none)
print(1 is numeric)
print((fn:0) is function)
print([] is list)
print("str" is string)
print(range(1) is range)

print(1 is 2)

loc class = fn p, init: {
	loc r = fn: {
		if p is function:
			loc n = p()
		else:
			loc n = { proto: p }
		init(n)
		n
	}

	r.proto = p

	r
}

loc Type0 = class({
	shout: fn: print(base.val)
}) { | self |
	self.val = "this is type0"
}

loc TypeA = class(Type0) { | self |
	self.val = "hi"
}

loc TypeB = class(Type0) {}

loc t1 = TypeA()
loc t2 = TypeB()

t1.shout()
t2.shout()

ret

// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "str: hi"
// -> "str: this is type0"
