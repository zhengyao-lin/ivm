
loc.merge(import class)

loc TypeA = class { | self |
	self.val = 10
	self.print = fn: print(self.val)
}

loc TypeB = class(TypeA()) { | self, val |
	self.val = val
	self.print = fn: print("b: " + self.val)
}

loc a = TypeA()
loc b = TypeB("hey")

a.print()
b.print()

// -> "num: 10"
// -> "str: b: hey"
