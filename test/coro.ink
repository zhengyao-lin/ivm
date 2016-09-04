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

	resume group: {
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
		resume g2 // g2 is locked
		print("never printed 2")
	}
}

g2 = group: {
	print("g2")
	resume g1
}

resume g2

g3 = group: {}

resume g3

print(resume g3 with "not executed")

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
// -> "num: 1"
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "num: 5"
// -> "num: 8"
// -> "num: 13"
// -> "num: 21"
// -> "num: 34"
// -> "num: 55"
// -> "num: 89"
// -> "num: 144"
// -> "num: 233"
// -> "num: 377"
// -> "num: 610"
// -> "num: 987"
// -> "num: 1597"
// -> "num: 2584"
// -> "num: 4181"
// -> "num: 6765"
// -> "num: 10946"
// -> "num: 17711"
// -> "num: 28657"
// -> "num: 46368"
// -> "num: 75025"
// -> "num: 121393"
// -> "num: 196418"
// -> "num: 317811"
// -> "num: 514229"
// -> "num: 832040"
// -> "num: 1346269"
// -> "str: printer end"
// -> "str: group test end"
// -> "str: end"
// -> "str: g2"
// -> "str: g3"
// -> "str: not executed"
