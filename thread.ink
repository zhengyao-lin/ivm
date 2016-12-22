import time

loc i = 0

__spawn(fork: {
	import test.bp
})

__spawn(fork: {
	import test
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