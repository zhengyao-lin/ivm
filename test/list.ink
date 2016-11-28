import std
import ulist

print([1, 2, 3, " happies"].reduce(fn a, b: a + b))

[1, 2, 3, 4, 5].filter(fn a: a % 2).print()

loc gen = fn n: {
	loc i = 1
	loc r = []

	while i <= n: {
		r.push(i)
		i = i + 1
	}

	r
}

loc primes = fn n: {
	loc r = gen(n).slice(1)
	loc denom = 2

	loc filter = fn e: e == denom || e % denom

	while denom < n: {
		r = r.filter(filter)
		denom = denom + 1
	}

	r
}

primes(100).print()

// -> "str: 6 happies"
// -> "str: \\[ 1, 3, 5 \\]"
// -> "str: \\[ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 \\]"

([ 1 ] * 3).print()
([ 1 ] * 0).print()

// -> "str: \\[ 1, 1, 1 \\]"
// -> "str: \\[\\]"
