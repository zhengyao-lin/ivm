
a = [1, 2, 3]
b = a.clone()

a[2] = "no!!"

for i in b:
	print(i)

// -> "num: 1"
// -> "num: 2"
// -> "num: 3"

gen = fn: {
	loc msg = "yes"
	fn: { print(msg) }
}

f1 = gen()
f2 = f1.clone()

del f1

i = 0
while i < 100000:
	i = i + 1

f2() // -> "str: yes"
