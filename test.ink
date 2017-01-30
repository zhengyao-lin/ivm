
fib = fn n: {
	if n < 2: ret 1
	ret fib(n - 1) + fib(n - 2)
}

print(fib(30))

ret

import mthread

loc i = 0

loc c = mthread.spawn(fork: {
	for loc j in range(1000000):
		i += 1

	ret "yeah"
})

for loc k in range(1000000): i += 1

mthread.join()

print(c.exitv())

print(i)

ret

import curses
import io
import test.ulist

loc tree = fn
	obj, log = [], indent = "",
	pslot = (
		fn k, v:
			indent + "   " +
			k + ": " + string(v) + " " +
			tree(v, log, indent + "   ")
	): {

	if log.has(obj): ret "{ ... }"
	log.push(obj)

	loc r = "{\n"
	loc has_slot = false

	if obj.proto: {
		has_slot = true
		r += pslot("proto", obj.proto)
	}

	for [ k, v ] in obj.slots(): {
		if has_slot: r += ",\n"
		has_slot = true
		r += pslot(k, v)
	}

	!has_slot ? "{}" : r + "\n" + indent + "}"
}

// object.proto.== = object.proto.==

print(tree(curses))
print(tree(io))

ret

import test.std

import time
import io
loc.merge(import curses)

try {

	initscr()
	raise exception()

	cbreak()
	// noecho()

	noqiflush()

	intrflush(false)
	stdscr.keypad(true)

	clear()

	loc w1 = window(5, 5, 5, 5)
	w1.addbg(attr.reverse)
	w1.addch("h")

	loc w2 = window(5, 5, 10, 10)
	w2.addbg(attr.reverse)
	w2.addch("h")

	refresh()

	time.msleep(3000)

	[ y, x ] = stdscr.pos()

	loc a = attr.bold | attr.underline

	move(5, 6)
	for i in range(10): {
		echoch("h", a)
		a ^= attr.underline
	}

	loc a = attr.reverse

	for i in range(5): {
		stdscr.addbg(a)
		refresh()
		time.msleep(300)

		stdscr.delbg(a)
		refresh()
		time.msleep(300)
	}
	
	refresh()

	getch()

} catch e: {
	print(e)
} final {
	endwin()
}

printe = fn e: {
	print("ERROR: " + string(e))
}

__test = fn {
	a = 1
	a.proto = string.proto

	print(a.+(1))

	string.proto.+ = "".+

	try print(a.+(1))
	catch none

	print(numeric("10"))
	print(numeric("10.01001"))
	print(numeric("-3.01"))
	try: print(numeric("-3.01e1"))
	catch: print("parse error")
	try: print(numeric("abcd"))
	catch: print("parse error")

	import test.ulist

	try: (fn: {
		raise exception("cannot find myself!")
	})()
	catch err: printe(err)

	import test.sort
	
	test.sort.bubble([5, 4, 3, 2, 1]).print()
	test.sort.qsort([5, 4, 3, 2, 1]).print()

	/*
	import test.testmod.sub
	import test.testmod.sub.a
	import test.testmod.sub.b

	test.testmod.sub.a.shout()
	test.testmod.sub.b.shout()

	import testmod
	*/

	print("测试宽字符串".len())

	変数1 = "この変数"
	print(変数1)

	变量1 = "这个变量"
	print(变量1)

	sort = $import("test/sort")

	try: $import("build/lib/wrongmod")
	catch err: printe(err)

	try: $import("build/lib/libivm-vm")
	catch err: printe(err)

	try: $import("never_found")
	catch err: printe(err)

	try: $import("wrongmod")
	catch err: printe(err)

	sort.bubble([5, 4, 3, 2, 1]).print()

	b = "no"

	(fn: {
		a = ref b
		deref a = "yes"

		print(deref a)
	})()

	print("123456".len())

	eval("print(\"from eval\")")
	print(eval("a"))
	print(eval("fn*,*:0"))

	print(eval("a = fn n: n < 2 && 1 || a(n - 1) + a(n - 2)")(10))

	f1 = fn: {
		(fn: {
			print("quite yes")
		})()
	}

	eval("(fn:fn:f1())()()")

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

	varg = fn a, *va, c: {
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
	loc print = { (): print }

	del print.()
	del print

	print = pb

	a = { val: 10, msg: "yes" }
	b = a.clone()
	i = 0

	while i < 10000: {
		i = i + 1
	}

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

	b = fn: {
		n = 10.clone()
		n.speak = fn a: print(a)
		n
	}

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

	loc f = none

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
	catch err: printe(err)

	try: print()
	catch err:
		try: print()

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

	try: 10 + 10

	if a > 11: print("yes!")
	else: print("no!")

	res = 0

	if a > 11: print("no!")
	else: print("yes!")

	print(a + 5)
	try: a * 5
	catch err: printe(err)

	try: none()
	catch err: printe(err)

	try: {(1 + "s")}
	catch err: printe(err)

	b = {}
	b.+ = fn a: print(base)

	b + 10
	b.+()

	try: {
		a = {}
		a.b = [].size
		a.b(1, 2)
	} catch err: {
		printe(err)
	}

	print((try: (fn:fn:fn:fn:raise "wonrg!!")()()()() catch none) == none)

	print("hello" == "hello")
	try: (fn:fn:fn:fn:raise "right!")()()()()
	catch err: print(err)

	i = 0

	print(!"hi")

	list = [2, 2, 3, 4, 5] + [2, 4, 5]
	a = 10
	i = 0

	while i < 10000000: {

		list[i] = a

		//print(list[2])
		i = i + 1
	}

	print("hey")
	print(list.size())

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

	func = fn n, b, a: {
		b = a + 10
		n = b + 1
		n + 1
		c = "hello, " + "world!"
		print(c)
	}

	func(1, { 2; 3 }, 4)

	print(if a < 11: "a < 11" elif a > 2: "a > 2" else: "no!")

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

	b = a.clone()

	b.b = "no!!"

	print(a.b)

	try: print(top a.b)
	catch: print("yes")

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
}

__test()
