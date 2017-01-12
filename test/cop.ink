import std
import ulist

a = {
	-->: fn v: {
		print("point to " + string(v))
		base
	},

	-----: fn: {
		print("split!")
		"split"
	},

	**: fn v: {
		print("don't say " + string(v) + "**!!")
		"**"
	},

	to_s: fn "<a>",

	noslot: fn slot: {
		loc b = base
		fn v: {
			print(slot + " " + string(v))
			b
		}
	}
}

a --> [ 1, 2, 3 ] --> 4

a --> ----- a

a ** "f"

a ***%%%@@@***|||** 10

a +++ a *** a

a *** a ??? a === a

object.proto.~= = fn val: {
	deref base = val
}

a = ref loc me

a ~= "yep"

print(me)

try 10 ~= 10
catch print("yeah")

object.proto.<= = fn obj: base.merge(obj) 

loc <= {
	another: "woho"
}

print(another)

ret

// -> "str: point to \\[ 1, 2, 3 \\]"
// -> "str: point to 4"
// -> "str: split!"
// -> "str: point to split"
// -> "str: don't say f\\*\\*!!"
// -> "str: \\*\\*\\*%%%\\@\\@\\@\\*\\*\\*|||\\*\\* 10"
// -> "str: \\*\\*\\* <a>"
// -> "str: \\+\\+\\+ <a>"
// -> "str: \\*\\*\\* <a>"
// -> "str: === <a>"
// -> "str: \\?\\?\\? <a>"
// -> "str: yep"

// -> "str: yeah"
// -> "str: woho"
