import std
import ulist
import math

// rule: [ from, char, next ]
loc dfa = fn rules, fin_node = [], init = 0: {
	loc tab = []
	// { a: "?" }

	for rule in rules: {
		loc from = rule[0]
		loc next = rule[2]

		if !tab[from]:
			tab[from] = {}

		if !tab[next]:
			tab[next] = {}

		tab[from][rule[1]] = next
	}

	loc fin = []

	for n in fin_node:
		fin[n] = true

	{
		add_final: fn node: {
			for n in fin:
				if n == node:
					ret
			fin.push(node)
		},

		accept: fn str: {
			loc cur = init
			loc i = 0
			loc len = str.len()

			while 1: {
				if i >= len:
					if fin[cur]:
						ret true
					else:
						ret false

				// print(tab[cur][str[i]])
				if (loc cur = tab[cur][str[i]]) == none: {
					// print(cur)
					ret false
				}

				i = i + 1
			}
		},

		// minimize DFA
		min: fn: {

		}
	}
}

d1 = dfa([
	[ 0, "a", 1 ],
	[ 1, "b", 2 ],
	[ 2, "c", 3 ],
	[ 3, "d", 4 ]
], [ 4 ])

print(d1.accept("abc"))
print(d1.accept("abcd"))
print(d1.accept("abcde"))

d2 = dfa([
	[ 0, "a", 1 ],
	[ 1, "a", 0 ]
], [ 1 ])

print(d2.accept("aa"))
print(d2.accept(""))
print(d2.accept("aaa"))
print(d2.accept("a"))
print(d2.accept("aaaa"))

print("number check")

loc d3 = dfa([
	[ 0, "0", 2 ],
	[ 0, ".", 4 ],

	[ 2, ".", 3 ],

	[ 0, "1", 1 ],
	[ 0, "2", 1 ],
	[ 0, "3", 1 ],
	[ 0, "4", 1 ],
	[ 0, "5", 1 ],
	[ 0, "6", 1 ],
	[ 0, "7", 1 ],
	[ 0, "8", 1 ],
	[ 0, "9", 1 ],

	[ 1, "0", 1 ],
	[ 1, "1", 1 ],
	[ 1, "2", 1 ],
	[ 1, "3", 1 ],
	[ 1, "4", 1 ],
	[ 1, "5", 1 ],
	[ 1, "6", 1 ],
	[ 1, "7", 1 ],
	[ 1, "8", 1 ],
	[ 1, "9", 1 ],

	[ 1, ".", 3 ],

	[ 3, "0", 3 ],
	[ 3, "1", 3 ],
	[ 3, "2", 3 ],
	[ 3, "3", 3 ],
	[ 3, "4", 3 ],
	[ 3, "5", 3 ],
	[ 3, "6", 3 ],
	[ 3, "7", 3 ],
	[ 3, "8", 3 ],
	[ 3, "9", 3 ],

	[ 4, "0", 3 ],
	[ 4, "1", 3 ],
	[ 4, "2", 3 ],
	[ 4, "3", 3 ],
	[ 4, "4", 3 ],
	[ 4, "5", 3 ],
	[ 4, "6", 3 ],
	[ 4, "7", 3 ],
	[ 4, "8", 3 ],
	[ 4, "9", 3 ]

], [ 1, 2, 3 ])

loc num_parse = fn str: {
	loc i = 0
	loc num = 0
	loc dec = 0

	loc rule = d3

	loc c2i = fn c: c.ord() - "0".ord()

	if !rule.accept(str):
		raise exception("failed to parse")

	for i in range(str.len()): {
		if str[i] == ".":
			dec = 1
		elif dec: {
			num = num + c2i(str[i]) / math.pow(10, dec)
			dec = dec + 1
		} else:
			num = num * 10 + c2i(str[i])
	}

	num
}

print(d3.accept("0.123"))
print(d3.accept("0."))
print(d3.accept("0"))
print(d3.accept("01"))
print(d3.accept("10023"))
print(d3.accept(".123"))
print(d3.accept("."))
print(d3.accept(".0"))
print(d3.accept("1.1."))
print(d3.accept("123131.2132131"))

print(num_parse("1.23"))
print(num_parse("123131.2"))
print(num_parse(".0"))
print(num_parse("0."))

ret

// -> "num: 0"
// -> "num: 1"
// -> "num: 0"
// -> "num: 0"
// -> "num: 0"
// -> "num: 1"
// -> "num: 1"
// -> "num: 0"

// -> "str: number check"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 0"
// -> "num: 1"
// -> "num: 1"
// -> "num: 0"
// -> "num: 1"
// -> "num: 0"
// -> "num: 1"

// -> "num: 1.23"
// -> "num: 123131.2"
// -> "num: 0"
// -> "num: 0"
