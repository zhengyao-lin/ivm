
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

print(a is TypeA)
print(b is TypeA)
print(a is TypeB)

// -> "num: 10"
// -> "str: b: hey"
// -> "num: 1"
// -> "num: 1"
// -> "num: 0"
