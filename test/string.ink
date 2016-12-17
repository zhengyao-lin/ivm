import std

try: "abcdefghi".ord(20)
catch: print("exception")

print("abcdefghi".ord())

try: print("".ord())
catch: print("yes")

print("".len())
print("22".len())

print("y".ord().char())

for loc c in "abcd".chars():
	print(c)

print("y" * 2)
print("y" * 0)
print("y" * 1)
print("y" * -100)
try: print("y" * 10000000000000000)
catch: print("yep")

ret

// -> "str: exception"
// -> "num: 97"
// -> "str: yes"
// -> "num: 0"
// -> "num: 2"
// -> "str: y"
// -> "str: a"
// -> "str: b"
// -> "str: c"
// -> "str: d"

// -> "str: yy"
// -> "str: "
// -> "str: y"
// -> "str: "
// -> "str: yep"
