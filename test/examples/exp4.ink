// all sorts of overloading...

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
string.proto.+? = fn c = base.ord(): c >= "A".ord() && c <= "Z".ord()
string.proto.-? = fn c = base.ord(): c >= "a".ord() && c <= "z".ord()

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
