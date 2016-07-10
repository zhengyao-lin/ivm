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

fib = fn n: {
	if n < 2: ret 1
	ret fib(n - 1) + fib(n - 2)
}

print(fib(40))

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

protof a
clone a

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
