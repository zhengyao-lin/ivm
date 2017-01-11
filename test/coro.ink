import std

f = fn: print("yeah")

resume fork: {
	for i in range(1000000): none
	f()
}

testo = {}

another = fork: {
	for i in range(100000): none
	testo.val = 10
}

c0 = fork: {
	print([ testo, resume another, 1 ][0].val)
}

resume c0

c1 = fork fn a: {
	print("arg: " + a)
	print("hi")
	yield "yield!"
	print("yah!")
}

print("huh?")
print(resume c1: 1)
resume c1
print("end")

try: resume coro.proto
catch: print("cannot resume")

dummy = fork: {
	while true: {
		for i in range(100000): none
		yield 10
	}
}

fib = fork: {
	loc a = 0
	loc b = 1

	while true: {
		[ 1, 2, 3, resume dummy, 4, 5 ]
		yield b
		loc c = a + b
		a = b
		b = c
	}
}

for i in range(100000): none
for i in range(100000): none

for i in range(20): {
	print(resume fib)
}

call(fn: try: yield catch: print("failed to yield"))

print("hi?")

loc gen = fn f: {
	loc c = fork f
	loc r = fn arg: {
		resume c: arg
	}

	r.iter = fn: {
		next: fn: r()
	}

	r
}

loc a = gen {
	loc val = 0
	while val < 20: {
		yield val * val
		val += 1
	}

	val * val
}

for i in a:
	print(i)

c = fork: print("right")

a = c.clone()
resume a

try: fork $import
catch: print("ok")

ret

// -> "str: yeah"
// -> "num: 10"
// -> "str: huh\\?"
// -> "str: arg: 1"
// -> "str: hi"
// -> "str: yield!"
// -> "str: yah!"
// -> "str: end"
// -> "str: cannot resume"
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
// -> "str: failed to yield"
// -> "str: hi."

// -> "num: 0"
// -> "num: 1"
// -> "num: 4"
// -> "num: 9"
// -> "num: 16"
// -> "num: 25"
// -> "num: 36"
// -> "num: 49"
// -> "num: 64"
// -> "num: 81"
// -> "num: 100"
// -> "num: 121"
// -> "num: 144"
// -> "num: 169"
// -> "num: 196"
// -> "num: 225"
// -> "num: 256"
// -> "num: 289"
// -> "num: 324"
// -> "num: 361"
// -> "num: 400"

// -> "str: right"
// -> "str: ok"
