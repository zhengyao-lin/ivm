
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
	i + {}
}
catch: print("outer")

print("passed")

// -> "str: something wrong"
// -> "str: inner"
// -> "str: mid"
// -> "str: outer"
// -> "str: passed"
