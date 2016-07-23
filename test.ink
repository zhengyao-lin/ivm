/*fib = fn n

:
fn n:

{
	if n < 2: ret 1

	ret fib(
		n - 1
	) +
	fib(
		n - 2
	)
}

fib(
	);
	;

	s-a*b+c-
	s;

a = b = fn:(
	a = b,
	a + 1,
	ret fn n : (n < 0),
	n+1,
	ret 0.23
)

fn:(10 + 2 + (fn:0))

a = fn:b = 2, c, d
c, b = fn n : ret n, ret 1

a = if a < b: {

}
elif a > b: (0, 0, 0)
else: c
*/
/*
0.proto.double = fn: base * 2

print(10.double())

TypeA = fn val: {
	proto: 0,
	val: val,
	print: fn: print(base.val),
	double: fn: val.double()
}

a = TypeA(1)

i = 0

// a.print()
while i < 1000000: {
	a.double()
	i = i + 1
}
*/

/*
a = {
	b: 10
}

i = 0
while i < 1000000: {
	// f()
	a["" + "b"] = 10
	i = i + 1
}

ret
 */


// a = i[1]

i = 0

list = [1, 2, 3, 4, 5] + [2, 4, 5]
a = 0
while i < 10000000: {

	list[i] = 10

	//print(list[2])
	i = i + 1
}

print("hi")

// list = []
ret

if 0: {
	i = 1

/*	while i < 1000000 && i > 0:
		if i == 2 && 0: i = 0
		elif 1:
			if 0 && 1: 0
			else: i = i + 1

	ret
*/
	print(1 || 0) // 1

	print(0 || 1) // 1

	print(0 && 1) // 0

	print(1 && 0) // 0

	print(0 && 0) // 0

	print(1 && 1) // 1

	print(1 && 1 || 1 && 0) // 1

	print(1 && 1 && 1 && 0) // 0

	print(1 && (1 || 0) && 1) // 1

	print(1 && (1 && 0) && 1) // 0

	print((1 && 0 || 0) && 1 || 1) // 1

	print((1 && ((1 && 1) && 0 || 0)) && 1 && 1) // 0

	if 1 && 1: print("yes!")
	if 1 && 0: print("no!")
	if 0 && 0: print("no!")
	if 1 && 0 || (1 && (0 || 1)): print("yes!")
	if 1 && (0 || (1 && (0 || 1))): print("yes!")

	// ret
}

/*
print({
	hey: "yes!!"
}["hey"])

ret
*/

fib = fn n: {
	if n < 2: ret 1
	ret fib(n - 1) + fib(n - 2)
}

print(fib(30))

ret

{
	b: 10,
	c: 10,
	d: if 0: a = 10
}

a = if 0: {
	n
} elif 1: {
	1
} else: {
	"str" + 1 + 8 * 9
}

//b = a.b.v - 1

a = 10

func = fn n, b, a: (
	b = a + 10,
	n = b + 1,
	n + 1,
	c = "hello, " + "world!",
	print(c)
)

func(1, (2, 3), 4)

print(if 0, a < 11: "a < 11" elif a > 2: "a > 2" else: "no!")

builder = fn n: {
	a: 10,
	b: 20,
	sum: fn: base.a + base.b
}

print(builder().sum())

i = 0

while i < 1000000: {
	// print(i)
	(fn:0)()
	i = i + 1
}

while 1: break

i = 0

while i < 10000: {
	if !(i % 100):
		print(i)
	i = i + 1
	while 1: 1
	cont 1
}

a = {
	dummy: 0,
	b: 2
}

b = clone a

b.b = "no!!"

print(a.b)

print(top a.b)

a = "yes!"
b = "no!"

func = fn: {
	loc a = "hey?"
	top b = "haha!"
}

print(func())

print(a + ", " + b)

func = fn: {
	loc = {
		a: "hola",
		b: 20,
		loc: "yes"
	}
	loc loc
}

print(func())

a = 10
a.proto.double = fn: base * 2

print(10.double())

TypeA = fn val: {
	proto: 0,
	val: val,
	print: fn: print(base.val),
	double: fn: val.double()
}

a = TypeA(1)

i = 0

// a.print()
a.double()
i = i + 1
