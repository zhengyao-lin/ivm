
err_func = fn: {
	raise "something wrong"
}

try: err_func()
catch err: print(err)

// -> "str: something wrong"
