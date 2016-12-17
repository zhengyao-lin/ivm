// object literal

import std
import ulist

_idxa_back = object.proto.[=]
object.proto.[=] = fn i, a: {
	if i is list: {
		if !(a is list):
			raise exception("failed to bond")
		for [ loc key, loc val ] in i.zip(a): base[key] = val
	} else: {
		_back = object.proto.[=]
		object.proto.[=] = _idxa_back
		base[i] = a
		object.proto.[=] = _back
	}

	a
}

a = {
	a: 10,
	.b: 11,
	+: "add oop",
	[]: "id assign oop",
	[ "hey" + "yo" ]: "hello",
	[ "first", "second" ]: [ 1, 2 ]
}

print(a.a)
print(a.b)
print(a.+)
print(a.[])
print(a.heyyo)
print(a.first)
print(a.second)

ret

// -> "num: 10"
// -> "num: 11"
// -> "str: add oop"
// -> "str: id assign oop"
// -> "str: hello"
// -> "num: 1"
// -> "num: 2"
