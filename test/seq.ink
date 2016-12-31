import std
import ulist

// sequence: objects with a callable .iter slot which returns a valid iterator 
// iterator: objects with a callable .next slot which returns a valid object and will throw an exception when the sequence is end

loc range10 = fn: {
	iter: {
		(): fn: range(10).iter()
	}
}

for i in range10():
	print(i)

[ a, b ] = range10()
print([ a, b ])

loc arith_seq = fn a, d, n = 10: { // u[n] = a + d * n 
	loc cur_n = 0
	{
		iter: fn: {
			next: fn: {
				if cur_n >= n:
					raise exception()
				else: {
					loc r = a + d * cur_n
					cur_n += 1
					r
				}
			}
		}
	}
}

print(list(arith_seq(1, 2, 4)))
[ a, , b, c ] = arith_seq(1, 10, 4)
print([ a, b, c ])

// generator
loc gen = fn core: {
	loc c = fork core

	fn: {
		iter: fn: {
			next: fn: {
				loc r = resume c
				assert c.alive()
				r
			}
		}
	}
}

loc g1 = gen {
	for i in range(, 100, 10):
		yield i
}

print(list(g1()))

ret

// -> "num: 0"
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "num: 5"
// -> "num: 6"
// -> "num: 7"
// -> "num: 8"
// -> "num: 9"
// -> "list: \\[ 0, 1 \\]"
// -> "list: \\[ 1, 3, 5, 7 \\]"
// -> "list: \\[ 1, 21, 31 \\]"
// -> "list: \\[ 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 \\]"
