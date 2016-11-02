printl = fn list: {
	loc size = list.size()
	loc i = 0

	print("### list ###")

	while i < size: {
		print(list[i])
		i = i + 1
	}

	print("### end ###")
}

t1 = fn a, *va, c: {
	print(a)
	printl(va)
	print(c)
}

t1("first", "last")
t1("first", "2", "3", "4", "last")

// -> "str: first"
// -> "str: ### list ###"
// -> "str: ### end ###"
// -> "str: last"
// -> "str: first"
// -> "str: ### list ###"
// -> "str: 2"
// -> "str: 3"
// -> "str: 4"
// -> "str: ### end ###"
// -> "str: last"
