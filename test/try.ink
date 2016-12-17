import ulist
import std

err_func = fn: {
	raise "something wrong"
}

try: err_func()
catch err: print(err)

try: 1 * [1]
try: 2 + p

try: {
	try: 1 + {}
	catch: print("inner")
	print("mid")
	1 + {}
} catch: print("outer")

try: {
	raise list(range(100))
} catch [ a, b, c ]: {
	print([ a, b, c ])
}

print("passed")

// -> "str: something wrong"
// -> "str: inner"
// -> "str: mid"
// -> "str: outer"
// -> "list: \\[ 0, 1, 2 \\]"
// -> "str: passed"
