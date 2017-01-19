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
