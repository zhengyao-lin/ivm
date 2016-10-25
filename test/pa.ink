import ulist

loc f = fn args...: {
	for loc i in args:
		print(typename(i))
}

pa = f(_, 2, _)

pa([], "hi")
// -> "str: list"
// -> "str: numeric"
// -> "str: string"

pa()
// -> "str: none"
// -> "str: numeric"
// -> "str: none"

pa("hi")
// -> "str: string"
// -> "str: numeric"
// -> "str: none"

pa(1, "hi", [], "hey", fn: 0)
// -> "str: numeric"
// -> "str: numeric"
// -> "str: string"

gen = fn count: {
	r = []
	i = 1

	while i <= count: {
		r.push(i)
		i = i + 1
	}

	r
}

loc power2 = fn a: a * a

for i in gen(50).apply(power2(_)):
	print(i)

// -> "num: 1"
// -> "num: 4"
// -> "num: 9"
// -> "num: 16"
// -> "num: 25"
// -> "num: 36"
// -> "num: 49"
// -> "num: 64"
// -> "num: 81"
// -> "num: 100"
// -> "num: 121"
// -> "num: 144"
// -> "num: 169"
// -> "num: 196"
// -> "num: 225"
// -> "num: 256"
// -> "num: 289"
// -> "num: 324"
// -> "num: 361"
// -> "num: 400"
// -> "num: 441"
// -> "num: 484"
// -> "num: 529"
// -> "num: 576"
// -> "num: 625"
// -> "num: 676"
// -> "num: 729"
// -> "num: 784"
// -> "num: 841"
// -> "num: 900"
// -> "num: 961"
// -> "num: 1024"
// -> "num: 1089"
// -> "num: 1156"
// -> "num: 1225"
// -> "num: 1296"
// -> "num: 1369"
// -> "num: 1444"
// -> "num: 1521"
// -> "num: 1600"
// -> "num: 1681"
// -> "num: 1764"
// -> "num: 1849"
// -> "num: 1936"
// -> "num: 2025"
// -> "num: 2116"
// -> "num: 2209"
// -> "num: 2304"
// -> "num: 2401"
// -> "num: 2500"

ret
