import ulist

loc types = fn lst: {
	if lst:
		for loc e in lst:
			print(typename(e))
}

loc f1 = fn *args: {
	types(args)
}

f1(, 2,)
print("###")
f1(, "wow", 2, )
print("###")
f1()
print("###")
f1(,)
print("###")
f1(,,)
print("###")
f1("hey", 2, [], "w")
print("###")
f1(,,,,,,)

print("#####")

[,].print()
[,,].print()
[].print()

[ a, ,b ] = [ 1, 2, 3 ]
[ a, b ].print()

[ ,,b ] = [ "no!", "hell", "yeah!" ]
print(b)

[ ,,, ] = [ 1, 2, 3 ]

[ ,b,, ] = [ 1, "match!", 3, 4 ]
print(b)

a = {
	[]: fn l: types(l)
}

print("###")
a[,,]
print("###")
a[,]
print("###")
a[]
print("###")

ret

// -> "str: none"
// -> "str: numeric"
// -> "str: none"
// -> "str: ###"
// -> "str: none"
// -> "str: string"
// -> "str: numeric"
// -> "str: none"
// -> "str: ###"
// -> "str: ###"
// -> "str: none"
// -> "str: none"
// -> "str: ###"
// -> "str: none"
// -> "str: none"
// -> "str: none"
// -> "str: ###"
// -> "str: string"
// -> "str: numeric"
// -> "str: list"
// -> "str: string"
// -> "str: ###"
// -> "str: none"
// -> "str: none"
// -> "str: none"
// -> "str: none"
// -> "str: none"
// -> "str: none"
// -> "str: none"
// -> "str: #####"
// -> "str: \\[ <none>, <none> \\]"
// -> "str: \\[ <none>, <none>, <none> \\]"
// -> "str: \\[\\]"
// -> "str: \\[ 1, 3 \\]"
// -> "str: yeah!"
// -> "str: match!"
// -> "str: ###"
// -> "str: none"
// -> "str: none"
// -> "str: none"
// -> "str: ###"
// -> "str: none"
// -> "str: none"
// -> "str: ###"
// -> "str: ###"
