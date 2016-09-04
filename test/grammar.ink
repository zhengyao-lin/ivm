[].proto.each = fn f: {
	for loc e in base: f(e)
}

[].proto.apply = fn f: {
	loc r = []
	for loc e in base:
		r.push(f(e))
	r
}

[].proto.zip = fn arr: {
	loc r = []
	loc s1 = base.size()
	loc s2 = arr.size()
	loc i = 0

	while i < s1 && i < s2: {
		r.push([base[i], arr[i]])
		i = i + 1
	}

	r
}

gen = fn count: {
	r = []
	i = 1

	while i <= count: {
		r.push(i)
		i = i + 1
	}

	r
}

loc a = gen(10)

for [a, b] in a zip(a apply to |x| x * x):
	print(a + " ^ 2 = " + b)

ret

// -> "str: 1 \\^ 2 = 1"
// -> "str: 2 \\^ 2 = 4"
// -> "str: 3 \\^ 2 = 9"
// -> "str: 4 \\^ 2 = 16"
// -> "str: 5 \\^ 2 = 25"
// -> "str: 6 \\^ 2 = 36"
// -> "str: 7 \\^ 2 = 49"
// -> "str: 8 \\^ 2 = 64"
// -> "str: 9 \\^ 2 = 81"
// -> "str: 10 \\^ 2 = 100"
