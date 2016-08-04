
err_func = fn: {
	raise "something wrong"
}

try: err_func()
catch err: print(err)

try: 1 * [1]
try: 2 + p

print("passed")

// -> "str: something wrong"
// -> "str: passed"
