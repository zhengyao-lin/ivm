import std

print("a " \
+ "continuation character")

a = {
	\: fn: {
		print("not a continuation character")
		"it's an operator"
	},

	\%%%\: fn: "also an operator",
	%%%\: fn: "don't override the one above"
}

// note that there is a space after \
print(a \ 
2)

print(a \%%%\
1)

ret

// -> "str: a continuation character"
// -> "str: not a continuation character"
// -> "str: it's an operator"
// -> "str: also an operator"
