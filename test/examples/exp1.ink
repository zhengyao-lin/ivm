// tolerable performance XD

loc fib = fn n:
	n < 2 ?
		1 : fib(n - 1) + fib(n - 2)

print(fib(30))
