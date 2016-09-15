
a = {}
b = {
	v1: "hi",
	v2: "hello"
}

print(a.merge(b).v2)
print(a.v1)

a = { -: fn: print("hello") }
b = { +: fn: print("yes") }

a.merge(b) + 1
a - 1

a = { val: "no overw" }
b = { val: "overw" }

print(a.clone().merge(b).val)
print(a.val)
print(a.merge(b, 1).val)

mir = loc
loc.merge({
	val1: "dummy1",
	val2: "dummy2",
	+: fn: print("loc add")
})

i = 0
while i < 1000000:
	i = i + 1

print(val1)
print(val2)
print(mir.val1)

mir + 10

ret

// -> "str: hello"
// -> "str: hi"
// -> "str: yes"
// -> "str: hello"
// -> "str: no overw"
// -> "str: no overw"
// -> "str: overw"
// -> "str: dummy1"
// -> "str: dummy2"
// -> "str: dummy1"
// -> "str: loc add"
