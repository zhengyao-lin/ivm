
import ulist

loc big_num = 1000000000000

try: a = [ 1 ] * big_num
catch: print("wrong1")

try: a = buffer(big_num)
catch: print("wrong2")

try: {
	a = [ 1, 2, 3 ]
	a[big_num] = 10
} catch: {
	print("wrong3")
	a.print()
}

ret

// -> "str: wrong1"
// -> "str: wrong2"
// -> "str: wrong3"
// -> "str: \\[ 1, 2, 3 \\]"
