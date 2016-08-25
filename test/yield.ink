wait = fn: {
	loc k = {}
	while (yield "haha") != "haha": 0
}

fork: {
	print("1")
	yield
	print("3")
}

fork: {
	print("2")
	yield
	print("4")
}

wait()
print(yield "end")

fork fn a: {
	print(a)
	yield "hi back"
	print("chat end")
}

print(yield "hi")
wait()
print("end")

print("#######")

call(fn: {
	try: {
		yield "hi"
		print("never printed")
	}

	yield to group: {
		fork: {
			loc a = 1
			loc b = 1
			loc c = a

			while yield a: {
				c = a + b
				a = b
				b = c
			}
		}

		// printer
		fork fn msg: {
			while msg: {
				print(msg)
				msg = yield 1
			}
			print("printer end")
			null
		}

		loc i = 0
		while i < 31: {
			yield 1
			i = i + 1
		}
		yield null
	}

	print("group test end")
})

print("end")

g1 = group: {
	print("g3")
	try: {
		yield to g2 // g2 is locked
		print("never printed 2")
	}
}

g2 = group: {
	print("g2")
	yield to g1
}

yield to g2

g3 = group: {

}

yield to g3

print(yield "not executed" to g3)

// -> "str: 1"
// -> "str: 2"
// -> "str: 3"
// -> "str: 4"
// -> "str: end"
// -> "str: hi"
// -> "str: hi back"
// -> "str: chat end"
// -> "str: end"
// -> "str: #######"
// -> "num: 1.000"
// -> "num: 1.000"
// -> "num: 2.000"
// -> "num: 3.000"
// -> "num: 5.000"
// -> "num: 8.000"
// -> "num: 13.000"
// -> "num: 21.000"
// -> "num: 34.000"
// -> "num: 55.000"
// -> "num: 89.000"
// -> "num: 144.000"
// -> "num: 233.000"
// -> "num: 377.000"
// -> "num: 610.000"
// -> "num: 987.000"
// -> "num: 1597.000"
// -> "num: 2584.000"
// -> "num: 4181.000"
// -> "num: 6765.000"
// -> "num: 10946.000"
// -> "num: 17711.000"
// -> "num: 28657.000"
// -> "num: 46368.000"
// -> "num: 75025.000"
// -> "num: 121393.000"
// -> "num: 196418.000"
// -> "num: 317811.000"
// -> "num: 514229.000"
// -> "num: 832040.000"
// -> "num: 1346269.000"
// -> "str: printer end"
// -> "str: group test end"
// -> "str: end"
// -> "str: g2"
// -> "str: g3"
// -> "str: not executed"
