loc printl = fn list: {
	loc i = 0
	loc size = list.size()

	print("### list ###")

	while i < size: {
		loc t = typeof(list[i])
		if t == "numeric" || t == "string":
			print(list[i])
		else:
			print("object of type <" + t + ">")
		i = i + 1
	}
}

[a, b, c] = [1, 2, 3]

printl([a, b, c])

i = 10
lst = []

while i < 10000: {
	lst.push(i * i)
	i = i + 1
}

[e, f, g, h] = [a, b, c] = lst

printl([a, b, c])
printl([e, f, g, h])

[a, a, a, a, a, a, a, a] = lst

printl([ a ])

a = "second"
b = "first"

[a, b] = [b, a]

printl([a, b])

[] = [1, 2]

// -> "str: ### list ###"
// -> "num: 1.000"
// -> "num: 2.000"
// -> "num: 3.000"
// -> "str: ### list ###"
// -> "num: 100.000"
// -> "num: 121.000"
// -> "num: 144.000"
// -> "str: ### list ###"
// -> "num: 100.000"
// -> "num: 121.000"
// -> "num: 144.000"
// -> "num: 169.000"
// -> "str: ### list ###"
// -> "num: 289.000"
// -> "str: ### list ###"
// -> "str: first"
// -> "str: second"
