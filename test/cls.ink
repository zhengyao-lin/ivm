
loc.merge(import class)

loc TypeA = class { | self |
	self.val = 10
	self.print = fn: print(self.val)
}

loc TypeB = class(TypeA) { | self |
	self.val = 11
	self.print = fn: print("b: " + self.val)
}

loc a = TypeA()
loc b = TypeB()

a.print()
b.print()

// -> "num: 10"
// -> "str: b: 11"
