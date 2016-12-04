import std

buf = buffer(8)
buf.init()

print(1.to_s())
print("hello".to_s())
print(buf.to_s())
print(type(1).to_s())

a = {
	val: 10,
	to_s: fn: {
		string(base.val + 10)
	}
}

print(string(a))

import ulist

print(string([ 1, 2, 3 ]))

try:
	print(string({
		to_s: fn: yield
	}))
catch:
	print("fatal")

print(string({
	to_s: fn: resume fork: {
		resume fork: {
			for i in range(1000000): none
			print("yes")
		}
		"great"
	}
}))

try: string({
	to_s: 1
})
catch: print("illegal to_s")

ret

// -> "str: 1"
// -> "str: hello"
// -> "str: 0x0000000000000000"
// -> "str: numeric"

// -> "str: 20"
// -> "str: \\[ 1, 2, 3 \\]"
// -> "str: fatal"
// -> "str: yes"
// -> "str: great"
// -> "str: illegal to_s"
