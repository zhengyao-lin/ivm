import std

a = 0
a += 1

print(a)

b = a += 1

print(a)
print(b)

a += a += 1

print(a)

a = {
	+=: fn n: {
		base.val += n
		base
	},
	-=: 0,
	*=: 0,
	/=: 0,
	val: 0,
	print: fn: {
		print(base.val)
	}
}

b = a

a += 1
a += 1

try: a /= 1
catch: print("yeah")

b.print()

print((1).+=(1))

ret

// -> "num: 1"

// -> "num: 2"
// -> "num: 2"
// -> "num: 5"
// -> "str: yeah"
// -> "num: 2"
// -> "num: 2"
