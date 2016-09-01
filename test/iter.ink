
list = []

for i in [0, 1, 2, 3, 4, 5]: {
	list.push(loc tmp = [])
	for j in [0, 1, 2, 3, 4, 5]: {
		tmp[j] = i * j
	}
}

for [a, b, c, d, e, f] in list: {
	print(a + ", " + b + ", " + c + ", " + d + ", " + e + ", " + f)
}

a = for i in [a, b, c]: 0

print(typeof(a))

list = []
i = 0

for list[i] in [0, 1, 2, 3, 4, 5]:
	i = i + 1

for i in list:
	print(i)

// -> "str: 0, 0, 0, 0, 0, 0"
// -> "str: 0, 1, 2, 3, 4, 5"
// -> "str: 0, 2, 4, 6, 8, 10"
// -> "str: 0, 3, 6, 9, 12, 15"
// -> "str: 0, 4, 8, 12, 16, 20"
// -> "str: 0, 5, 10, 15, 20, 25"
// -> "str: none"
// -> "num: 0.000"
// -> "num: 1.000"
// -> "num: 2.000"
// -> "num: 3.000"
// -> "num: 4.000"
// -> "num: 5.000"
