a = "no!"
f = fn: {
	loc a = "wow"
	del a
	a = "yes!"
}
i = 0

while i < 100000: {
	f()
	i = i + 1
}

print(a) // -> "str: yes!"

a = "outermost"

f1 = fn: {
	loc a = "out"
	f2 = fn: {
		a = "inside"
	}
	f2()
	print(a) // -> "str: inside"
	del a
	print(a) // -> "str: outermost"
}

f1()
