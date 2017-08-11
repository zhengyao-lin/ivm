# ivm
ivm is a simple vm built for a prototype-based language [ink](https://github.com/rod-lin/ink "ink")
<br>
### Prerequisites
    cmake >= 2.8
	gcc >= 4.8.4 or clang >= 3.0

### Build
Build using cmake

	cmake -DVERSION=release
	make

### After Building
run tests or anything you write

	make test
	build/bin/ink hello.ink

### Examples(of ink)
ink is a dynamically-typed language with...

#### Weird grammar
	
	// js-like prototype mechanism
	list.proto.map = fn f:
		[ f(i) for loc i in base ]

	// partial applied function
	[ 1, 2, 3 ].
		map(1 .+(_)).
		map {
			i -> i * 2
		}.
		map(print)


#### Natively supported coroutine

	loc c = fork fn [ (yield i * 2) for loc i in range(10) ]
	
	while c.alive():
		print(resume c)


#### All sorts of overloading(which is bad)...

	// context overload
	loc = loc.clone()

	list.proto.reduce = fn f: {
		assert base.size() > 1

		loc init = base[0]
		
		for loc e in base.slice(1):
			init = f(init, e)

		init
	}

	list.proto.map = fn f: [ f(i) for loc i in base ]

	// is capital or non-capital letter
	// custom operators
	string.proto.+? =
		fn c = base.ord(): c >= "A".ord() && c <= "Z".ord()
	string.proto.-? =
		fn c = base.ord(): c >= "a".ord() && c <= "z".ord()

	string.proto.~ = fn:
		base.
			chars().
			map(fn c:
					if +? c:
						c.ord() - "A".ord() + "a".ord()
					elif -? c:
						c.ord() - "a".ord() + "A".ord()
					else:
						c.ord()
				).
			map(fn c: c.char()).
			reduce(fn i, a: i + a)

	print(~"hELLO, WORLD!")

#### ... And tolerable performance
	
	loc fib = fn n:
		n < 2 ?
			1 : fib(n - 1) + fib(n - 2)

	print(fib(30))

more tests and examples can be found in test and test/examples folders
