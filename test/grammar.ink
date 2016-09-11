import ulist

gen = fn count: {
	r = []
	i = 1

	while i <= count: {
		r.push(i)
		i = i + 1
	}

	r
}

loc a = gen(10)

for [a, b] in a zip(a apply to |x| x * x):
	print(a + " ^ 2 = " + b)

for i in [ 1, 2, 2, 3, 1, "hi", "wow", "wow", "hi", 3, 3, 5, 6, 6, 9, 1, 5, 6, 7 ].uniq():
	print(i)

for i in gen(100000).reverse():
	if !(i % 1000):
		print(i)

ret

// -> "str: 1 \\^ 2 = 1"
// -> "str: 2 \\^ 2 = 4"
// -> "str: 3 \\^ 2 = 9"
// -> "str: 4 \\^ 2 = 16"
// -> "str: 5 \\^ 2 = 25"
// -> "str: 6 \\^ 2 = 36"
// -> "str: 7 \\^ 2 = 49"
// -> "str: 8 \\^ 2 = 64"
// -> "str: 9 \\^ 2 = 81"
// -> "str: 10 \\^ 2 = 100"

// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "str: hi"
// -> "str: wow"
// -> "num: 5"
// -> "num: 6"
// -> "num: 9"
// -> "num: 7"
// -> "num: 100000"
// -> "num: 99000"
// -> "num: 98000"
// -> "num: 97000"
// -> "num: 96000"
// -> "num: 95000"
// -> "num: 94000"
// -> "num: 93000"
// -> "num: 92000"
// -> "num: 91000"
// -> "num: 90000"
// -> "num: 89000"
// -> "num: 88000"
// -> "num: 87000"
// -> "num: 86000"
// -> "num: 85000"
// -> "num: 84000"
// -> "num: 83000"
// -> "num: 82000"
// -> "num: 81000"
// -> "num: 80000"
// -> "num: 79000"
// -> "num: 78000"
// -> "num: 77000"
// -> "num: 76000"
// -> "num: 75000"
// -> "num: 74000"
// -> "num: 73000"
// -> "num: 72000"
// -> "num: 71000"
// -> "num: 70000"
// -> "num: 69000"
// -> "num: 68000"
// -> "num: 67000"
// -> "num: 66000"
// -> "num: 65000"
// -> "num: 64000"
// -> "num: 63000"
// -> "num: 62000"
// -> "num: 61000"
// -> "num: 60000"
// -> "num: 59000"
// -> "num: 58000"
// -> "num: 57000"
// -> "num: 56000"
// -> "num: 55000"
// -> "num: 54000"
// -> "num: 53000"
// -> "num: 52000"
// -> "num: 51000"
// -> "num: 50000"
// -> "num: 49000"
// -> "num: 48000"
// -> "num: 47000"
// -> "num: 46000"
// -> "num: 45000"
// -> "num: 44000"
// -> "num: 43000"
// -> "num: 42000"
// -> "num: 41000"
// -> "num: 40000"
// -> "num: 39000"
// -> "num: 38000"
// -> "num: 37000"
// -> "num: 36000"
// -> "num: 35000"
// -> "num: 34000"
// -> "num: 33000"
// -> "num: 32000"
// -> "num: 31000"
// -> "num: 30000"
// -> "num: 29000"
// -> "num: 28000"
// -> "num: 27000"
// -> "num: 26000"
// -> "num: 25000"
// -> "num: 24000"
// -> "num: 23000"
// -> "num: 22000"
// -> "num: 21000"
// -> "num: 20000"
// -> "num: 19000"
// -> "num: 18000"
// -> "num: 17000"
// -> "num: 16000"
// -> "num: 15000"
// -> "num: 14000"
// -> "num: 13000"
// -> "num: 12000"
// -> "num: 11000"
// -> "num: 10000"
// -> "num: 9000"
// -> "num: 8000"
// -> "num: 7000"
// -> "num: 6000"
// -> "num: 5000"
// -> "num: 4000"
// -> "num: 3000"
// -> "num: 2000"
// -> "num: 1000"