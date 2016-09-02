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

i = -1
l = []
r = fn: ref l[i = i + 1]

[[deref r(), deref r()], deref r(), [deref r(), [deref r(), deref r()], deref r()]] = [[1, 2], 3, [4, [5, 6], 7]]

for i in l:
	print(i)

// -> "str: ### list ###"
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "str: ### list ###"
// -> "num: 100"
// -> "num: 121"
// -> "num: 144"
// -> "str: ### list ###"
// -> "num: 100"
// -> "num: 121"
// -> "num: 144"
// -> "num: 169"
// -> "str: ### list ###"
// -> "num: 289"
// -> "str: ### list ###"
// -> "str: first"
// -> "str: second"
// 
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "num: 5"
// -> "num: 6"
// -> "num: 7"
