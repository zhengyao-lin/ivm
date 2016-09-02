fib = fn n: {
	if n < 2: ret 1
	ret fib(n - 1) + fib(n - 2)
}

print(fib(25)) // -> "num: 121393"
