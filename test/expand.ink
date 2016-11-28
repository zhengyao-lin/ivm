import std
import ulist

loc f = fn *args: {
	args.print()
	10011
}

print(2 + f(expand [ 1, 2, 3 ], expand [ 1, 2 ]))

loc a = {
	f: f
}

print(4 + a.f(expand [ "wow", "hey" ], expand [ 1, 2, 3 ]))

p = a.f(_, expand [ 1, 2, 3 ], _, expand [ "hi", "wow" ], _, _)

print(10 + p(1000, 20, 0, 5))

try: f(expand 1)
catch: print("error")

f(expand [])

print([ "yeah", 2 ][(for loc i in range(10): {
	f(1, f(expand [ break ]), 1)
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
