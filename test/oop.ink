import std
import io

print(io.file.proto == 1) // -> "num: 0"
print(none == 1) // -> "num: 0"
print(none != 1) // -> "num: 1"
print("s" == 1) // -> "num: 0"
print("s" != 1) // -> "num: 1"

object.proto.! = fn: print("no!!!")

print(!0)
print((0).!())

print("hello".+ == "hi".+)

object.proto.+ = fn: print("hi")

"hello, " + {}
"hello, ".+({})

a = 10
a.+ = fn b: base - b
a.* = "hi"

print(a + 5)
try: a * 5
catch err: print("failed")

res = 1

a.<= = fn b: res
a.>= = fn b: a <= b
a.< = fn b: a >= b
a.> = fn b: a < b

try: 10 + ""

if a > 11: print("yes!")
else: print("no!")

res = 0

if a > 11: print("no!")
else: print("yes!")

a = [1, 2, 3]

a.[] = fn idx: {
	if typename(idx) == "list":
		for i in idx:
			print(i)
	else:
		print(typename(idx))
}

a[1, 2, 3]
a[]

a = {
	(): fn: {
		print("a is called")
	}
}

a()

a = {
	val: 0,
	[]: fn idx: {
		print("here")
		base.val
	},

	[=]: fn idx, assign: {
		print("there")
		if assign != none:
			base.val = assign
		else:
			base.val
	}
}

print(a["anything"])
print(a["anything"] = 10)

print(a["anything"])

import ulist

print((1).+(2))
print([ 1 ].*(5).to_str())
try: print("str".[](5).to_str())
catch: print("yes")

a = [ 1, 2, 3 ]

a.[=](2, "hi")

print(a.[](2))

f = a.[=]
b = { [=]: f }

try: b[10] = 10
catch: print("hey, wrong base")

print((1).+(10))
print((2).-(20))
print((3).*(40))
print((3)./(10))

print((3).>(10))
print((3).<(2))
print((3).&(10))

print((3).%(10))
print([ 1 ].+([ 2 ]).to_str())

f = (fn: print("called! base: " + base.val))
f.val = "yeah"
a = { f: f, val: "no!" }

a.f.()()

~{
	~: fn: {
		print("wow")
	}
}

object.proto.proto = {
	proto: none,
	[=]: fn k, v: {
		print(k)
		print(v)
	}
}

{}[1] = "hi"
{}.[=](2, "hi")

del string.proto.*

try "hi" * 2 catch print("failed")

string.proto.* = fn print("no!")
del string.proto.*

try "hi" * 2 catch print("failed")

numeric.proto.% = 2
del numeric.proto.%

try print(1 % 2) catch print("yeah")

// -> "num: 1"
// -> "num: 1"
// -> "num: 1"

// -> "str: hi"
// -> "str: hi"

// -> "num: 5"
// -> "str: failed"
// -> "str: yes!"
// -> "str: yes!"
// 
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
//
// -> "str: none"
// -> "str: a is called"
//
// -> "str: here"
// -> "num: 0"
// -> "str: there"
// -> "num: 10"
// -> "str: here"
// -> "num: 10"

// -> "num: 3"
// -> "str: \\[ 1, 1, 1, 1, 1 \\]"
// -> "str: yes"

// -> "str: hi"
// -> "str: hey, wrong base"
// -> "num: 11"
// -> "num: -18"
// -> "num: 120"
// -> "num: 0.3"
// -> "num: 0"
// -> "num: 0"
// -> "num: 2"
// -> "num: 3"
// -> "str: \\[ 1, 2 \\]"

// -> "str: called! base: yeah"
// 
// -> "str: wow"

// -> "num: 1"
// -> "str: hi"
// -> "num: 2"
// -> "str: hi"

// -> "str: failed"
// -> "str: failed"

// -> "str: yeah"
