a = fn: {
	ret fn: {
		print(the_msg) // -> "str: hey"
	}
}

a = a()

loc = {
	the_msg: "hey",
	a: a,
	print: print
}

a()

a = fn: {
	a = (fn: {
		ret fn: print(hello) // -> "str: hi"
	})()
	hello = "hi"
	a()
}

a()
