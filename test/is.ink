print(none is none)
print(1 is numeric)
print((fn:0) is function)
print((fn:0) is numeric)
print([] is list)
print("str" is string)
print(range(1) is range)

print(1 is 2)

loc class = fn p, init: {
	if typeof(p) == "function":
		p = p()

	loc r = fn: {
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

print(t1 is numeric)
print(t1 is object)
print(t1 is Type0)
print(t1 is TypeA)
print(t1 is function)
print(t1 is TypeB)

t2.shout()

ret

// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 0"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "str: hi"
// -> "num: 0"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 0"
// -> "num: 0"
// -> "str: this is type0"
