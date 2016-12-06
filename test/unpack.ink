import std

loc printl = fn list: {
	loc i = 0
	loc size = list.size()

	print("### list ###")

	while i < size: {
		loc t = typename(list[i])
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

import ulist

[ a, *b, c ] = [ 1, 2, 3, 2 ]
[ a, b, c ].print()

[ a, *[ [ b, c, d ], e, f ], g ] = [ 1, [ 2, 3, 4 ], 5, 6, 7 ]
[ a, b, c, d, e, f, g ].print()

[ a, *[ *[ b, c, d ], e, f ], g ] = [ 1, 2, 3, 4, 5, 6, 7 ]
[ a, b, c, d, e, f, g ].print()

[ a, *[ *[ b, c, d ], e, f ], g, h, i ] = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ]
[ a, b, c, d, e, f, g, h, i ].print()


[ a, *[ *[ b, c, d ], e, f ], g ] = [ a, *[ *[ b, c, d ], e, f ], g, h, i ] = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ]
[ a, b, c, d, e, f, g, h, i ].print()

r = ref a

[ a, *[ *[ b, c, d ], deref r, f ], g, h, i ] = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ]
[a].print()

[ *deref r ] = [ 1, 2, 3, 4 ]
a.print()

loc n_sum = fn l: {
	[ t, *h ] = l
	print(t)
	h.print()
	if t == none: 0
	else: t + n_sum(h)
}

// print(n_sum([ 1, 2, 3, 4 ]))

[ t, *h, e ] = [ 1 ]
[ t, h, e ].print()

loc fib = fn n: {
	iter: fn: {
		a: 0,
		b: 0,
		cur: 0,
		next: fn: {
			if n <= base.cur: raise "iter end"
			else: {
				loc tmp = base.a + base.b
				base.a = base.b
				base.b = tmp
				base.cur = base.cur + 1
				
				if base.b == 0:
					base.b = 1
				else:
					tmp
			}
		}
	}
}

[ a, b, c, *d ] = fib(15)

print([ a, b, c, d ])

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

// -> "str: \\[ 1, \\[ 2, 3 \\], 2 \\]"
// -> "str: \\[ 1, 2, 3, 4, 5, 6, 7 \\]"
// -> "str: \\[ 1, 2, 3, 4, 5, 6, 7 \\]"
// -> "str: \\[ 1, 2, 3, 4, 5, 6, 7, 8, 9 \\]"
// -> "str: \\[ 1, 2, 3, 4, 7, 8, 9, 8, 9 \\]"
// -> "str: \\[ 5 \\]"
// -> "str: \\[ 1, 2, 3, 4 \\]"

// -> "str: \\[ 1, \\[\\], <none> \\]"

// -> "list: \\[ 1, 1, 2, \\[ 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610 \\] \\]"
