// pseudo iter

list_proto = [].proto

list_proto.iter = fn: {
	list: base,
	cur: 0,
	next: fn: {
		if base.cur < base.list.size():
			loc val = base.list[base.cur]
		else:
			raise "done"

		base.cur = base.cur + 1

		val
	}
}

list = []
i = 0

while i < 100000: {
	list.push(i)
	i = i + 1
}

a = list.iter()

while 1: {
	try: i = a.next()
	catch err:
		break
	if !(i % 1000): {
		print(i)
	}
}

// -> "num: 0"
// -> "num: 1000"
// -> "num: 2000"
// -> "num: 3000"
// -> "num: 4000"
// -> "num: 5000"
// -> "num: 6000"
// -> "num: 7000"
// -> "num: 8000"
// -> "num: 9000"
// -> "num: 10000"
// -> "num: 11000"
// -> "num: 12000"
// -> "num: 13000"
// -> "num: 14000"
// -> "num: 15000"
// -> "num: 16000"
// -> "num: 17000"
// -> "num: 18000"
// -> "num: 19000"
// -> "num: 20000"
// -> "num: 21000"
// -> "num: 22000"
// -> "num: 23000"
// -> "num: 24000"
// -> "num: 25000"
// -> "num: 26000"
// -> "num: 27000"
// -> "num: 28000"
// -> "num: 29000"
// -> "num: 30000"
// -> "num: 31000"
// -> "num: 32000"
// -> "num: 33000"
// -> "num: 34000"
// -> "num: 35000"
// -> "num: 36000"
// -> "num: 37000"
// -> "num: 38000"
// -> "num: 39000"
// -> "num: 40000"
// -> "num: 41000"
// -> "num: 42000"
// -> "num: 43000"
// -> "num: 44000"
// -> "num: 45000"
// -> "num: 46000"
// -> "num: 47000"
// -> "num: 48000"
// -> "num: 49000"
// -> "num: 50000"
// -> "num: 51000"
// -> "num: 52000"
// -> "num: 53000"
// -> "num: 54000"
// -> "num: 55000"
// -> "num: 56000"
// -> "num: 57000"
// -> "num: 58000"
// -> "num: 59000"
// -> "num: 60000"
// -> "num: 61000"
// -> "num: 62000"
// -> "num: 63000"
// -> "num: 64000"
// -> "num: 65000"
// -> "num: 66000"
// -> "num: 67000"
// -> "num: 68000"
// -> "num: 69000"
// -> "num: 70000"
// -> "num: 71000"
// -> "num: 72000"
// -> "num: 73000"
// -> "num: 74000"
// -> "num: 75000"
// -> "num: 76000"
// -> "num: 77000"
// -> "num: 78000"
// -> "num: 79000"
// -> "num: 80000"
// -> "num: 81000"
// -> "num: 82000"
// -> "num: 83000"
// -> "num: 84000"
// -> "num: 85000"
// -> "num: 86000"
// -> "num: 87000"
// -> "num: 88000"
// -> "num: 89000"
// -> "num: 90000"
// -> "num: 91000"
// -> "num: 92000"
// -> "num: 93000"
// -> "num: 94000"
// -> "num: 95000"
// -> "num: 96000"
// -> "num: 97000"
// -> "num: 98000"
// -> "num: 99000"
