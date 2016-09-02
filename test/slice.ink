lst = [1, 2, 3, 4, 5, 6, 7, 8, 9]

printl = fn list: {
	print(loc size = list.size())
	loc i = 0

	while i < size: {
		print(list[i])
		i = i + 1
	}
}

printl(lst)
print("###")
printl(lst.slice(1, 9))
print("###")
printl(lst.slice(1, 8))
print("###")
printl(lst.slice(1, -1))
print("###")
printl(lst.slice(-1, 0))
print("###")
printl(lst.slice(8, 0))
print("###")
printl(lst.slice(9, 0))

print("###")
printl(lst.slice(1, 8, 1))
print("###")
printl(lst.slice(1, 8, -1))
print("###")
printl(lst.slice(8, 1, -1))
print("###")
printl(lst.slice(8, 1, 1))

print("###")
printl(lst.slice(8, 1, -2))
print("###")
printl(lst.slice(8, 1, -3))

print("###")
printl(lst.slice(1, 9, 2))
print("###")
printl(lst.slice(1, 9, 8))

print("###")
printl(lst.slice(1, 9, 2.7))
print("###")
printl(lst.slice(1, 9, 8.9))

// -> "num: 9"
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "num: 5"
// -> "num: 6"
// -> "num: 7"
// -> "num: 8"
// -> "num: 9"
// -> "str: ###"
// -> "num: 8"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "num: 5"
// -> "num: 6"
// -> "num: 7"
// -> "num: 8"
// -> "num: 9"
// -> "str: ###"
// -> "num: 7"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "num: 5"
// -> "num: 6"
// -> "num: 7"
// -> "num: 8"
// -> "str: ###"
// -> "num: 7"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "num: 5"
// -> "num: 6"
// -> "num: 7"
// -> "num: 8"
// -> "str: ###"
// -> "num: 0"
// -> "str: ###"
// -> "num: 0"
// -> "str: ###"
// -> "num: 0"
// -> "str: ###"
// -> "num: 7"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "num: 5"
// -> "num: 6"
// -> "num: 7"
// -> "num: 8"
// -> "str: ###"
// -> "num: 0"
// -> "str: ###"
// -> "num: 7"
// -> "num: 9"
// -> "num: 8"
// -> "num: 7"
// -> "num: 6"
// -> "num: 5"
// -> "num: 4"
// -> "num: 3"
// -> "str: ###"
// -> "num: 0"
// -> "str: ###"
// -> "num: 4"
// -> "num: 9"
// -> "num: 7"
// -> "num: 5"
// -> "num: 3"
// -> "str: ###"
// -> "num: 3"
// -> "num: 9"
// -> "num: 6"
// -> "num: 3"
// -> "str: ###"
// -> "num: 4"
// -> "num: 2"
// -> "num: 4"
// -> "num: 6"
// -> "num: 8"
// -> "str: ###"
// -> "num: 1"
// -> "num: 2"
// -> "str: ###"
// -> "num: 4"
// -> "num: 2"
// -> "num: 4"
// -> "num: 6"
// -> "num: 8"
// -> "str: ###"
// -> "num: 1"
// -> "num: 2"
