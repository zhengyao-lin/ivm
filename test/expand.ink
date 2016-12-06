import std
import ulist

loc f = fn *args: {
	args.print()
	10011
}

print(2 + f(*[ 1, 2, 3 ], *[ 1, 2 ]))

loc a = {
	f: f
}

print(4 + a.f(*[ "wow", "hey" ], *[ 1, 2, 3 ]))

p = a.f(_, *[ 1, 2, 3 ], _, *[ "hi", "wow" ], _, _)

print(10 + p(1000, 20, 0, 5))

try: f(*1)
catch: print("error")

f(*[])

print([ "yeah", 2 ][(for loc i in range(10): {
	f(1, f(*[ break ]), 1)
}, 0)])

// -> "str: \\[ 1, 2, 3, 1, 2 \\]"
// -> "num: 10013"
// -> "str: \\[ wow, hey, 1, 2, 3 \\]"
// -> "num: 10015"
// -> "str: \\[ 1000, 1, 2, 3, 20, hi, wow, 0, 5 \\]"
// -> "num: 10021"
// -> "str: error"
// -> "str: \\[\\]"
// -> "str: yeah"
