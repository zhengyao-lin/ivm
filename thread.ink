import time

loc i = 0

__spawn(fork: {
	print("######################### bp")
	import test.bp
})

__spawn(fork: {
	print("######################### test")
	import test
})

__spawn(fork: {
	print("######################### sort")
	import test.sort
})

__spawn(fork: {
	print("######################### gc")
	import test.gc
})

__spawn(fork: {
	print("######################### testm")
	import test.testm
})

__spawn(fork: {
	print("######################### ga")
	import test.ga
})

__spawn(fork: {
	fib = fn n: {
		if n < 2: ret 1
		ret fib(n - 1) + fib(n - 2)
	}

	print(fib(30))
})

// while end != 2: none

print(i)

ret