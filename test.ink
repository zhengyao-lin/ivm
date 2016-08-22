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
/*
gid = group: {
	fork fn b: {

	}

	fork fn a: {

	}

	while yield: 0
}

yield a to gid
*/

a = "no!"
f = fn: {
	loc a = "wow"
	del a
	a = "yes!"
}
i = 0

while i < 100000: {
	f()
	i = i + 1
}

print(a)

ret

print("123456".len())

eval("print(\"from eval\")")
print(eval("a"))
print(eval("fn...,...:0"))

print(eval("a = fn n: n < 2 && 1 || a(n - 1) + a(n - 2)")(10))

printl = fn list: {
	loc size = list.size()
	loc i = 0

	print("*** list ***")

	while i < size: {
		print(list[i])
		i = i + 1
	}

	print("*** end ***")
}

varg = fn a, va..., c: {
	print(a)
	printl(va)
	print(va[0])
	print(c)
}

varg(1, 2, 3, 4)

print("hi " + 100000000000000000000000000000000001)
print(100000000000000000000000000000000001 + " wow")

print("abcdefg"[3])

pb = print
print = { (): print }

del print.()
del print

print = pb

a = { val: 10, msg: "yes" }
b = clone a
i = 0

while i < 10000:
	i = i + 1

a.msg = "no"

print(b.msg)

c = {
	+: fn: print("c.+ called")
}

a = {
	proto: c, 
	-: fn b: print("a.- called"),
	-@: fn: print("a.-@ called")
}

b = { proto: a }
b + 1
b - 1
-b

print("hi")

b = fn: (
	n = clone 10,
	n.speak = fn a: print(a),
	n
)

a = b()
a.speak("yes")

a = {
	val: "yes",
	(): fn: {
		print("i was called")
		print(base.val)
	}
}

a()

b = {
	val: "no",
	a: a
}

b.a()

gid = group fn a: {
	print("group")
	print(a)
	"from group"
}

fork fn a: {
	print("fork")
	print(a)
}

yield yield 10 to gid

i = 0
f = null

(fn: {
	f = fn: print(val)

	while i < 10000:
		i = i + 1

	loc = { val: "wow" }
})()

while i < 100000:
	i = i + 1

f()

a = 0
b = 0
c = 0
a.proto = b
c.proto = a
try: b.proto = c
catch err: print(err.msg)

try: print()
catch err: try: print()

// a = i[1]
a = 10
b = {}

a.+ = fn b: base - b
a.* = "hi"

a.>> = fn b: b.value = a

print(1 << 2)
print(1024 >> 2)
print(-1 >> 2)
print(-1 >>> 1)

a >> b
print(b.value >> a)
print(a.value)

res = 1

a.<= = fn b: res
a.>= = fn b: a <= b
a.< = fn b: a >= b
a.> = fn b: a < b

try: 10 + ""

if a > 11: print("yes!")
else: print("no!")

res = 0

if a > 11: print("no!")
else: print("yes!")

print(a + 5)
try: a * 5
catch err: print(err.msg)

try: null()
catch err: print(err.msg)

try: {(1 + "s")}
catch err: print(err.msg)

b = {}
b.+ = fn a: print(base)

b + 10
b.+()

try: {
	a = {}
	a.b = [].size
	a.b(1, 2)
} catch err: {
	print(err.msg)
}

print((try: (fn:fn:fn:fn:raise "wonrg!!")()()()()) == null)
print("hello" == "hello")
try: (fn:fn:fn:fn:raise "right!")()()()()
catch err: print(err)

print(call(fn:(yield "ok!")))

i = 0

fork fn a: {
	print("init " + a)
	while a = yield null:
		if a == "skip":
			print("skip")
		else: {
			print("received " + a)
		}
	print("printer end")
}

yield "1"
yield "2"
yield "3"
yield "4"
yield "skip"
yield "5"

gid = 0

call(fn: {
	call(fn: {
		yield "6"
		yield to gid = group: {
			fork: {
				print("from group: 1")
				yield 1
				print("from group: 3")
				//print("hi")
			}

			fork: {
				print("from group: 2")
				yield 1
				print("from group: 4")
			}

			while yield null: 0
			print("end of the group")
			yield to 0
			print("very end")
		}
		yield "7"
	})
	print("#this is 8")
	yield "9"
})

yield null

print("stop fork")

fork: {
	call((fn: (yield 1, print("2?"))), "hi")
}

//yield null

call(fn: (yield 2, print("1?")))

yield to gid

///////////////////////////////////////

fork: {
	yield to gid = group: {
		print("another group")
		yield to 0
		print("never printed")
	}
	print("not next?")
}

yield
yield to gid

print(!"hi")

list = [2, 2, 3, 4, 5] + [2, 4, 5]
a = 10
while i < 10000000: {

	list[i] = a

	//print(list[2])
	i = i + 1
}

print("hey")
print(list.size())

//ret
// list = []

if 0: {
	i = 1

/*	while i < 1000000 && i > 0:
		if i == 2 && 0: i = 
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

{
	b: "find me!!!!!",
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
while i < 100000: {
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
