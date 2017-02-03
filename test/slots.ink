/**
 * 3 types of slot:
 *     1. normal slot
 *            a. id slot
 *            b. op slot
 *     2. proto slot
 *
 * for normal slot:
 *
 *     obj.id or obj["id"]
 * 
 *     GET order:
 *         obj.id
 *         obj.proto.id
 *         obj.proto.proto.id
 *         obj.proto.proto.proto....id until the proto slot is empty. If so, return none
 *
 *     SET order:
 *         obj.id = 10(directly set on the uppermost object)
 *
 *     DELETE order: same as SET order
 *
 * for proto slot:
 *
 *     obj.proto or obj["proto"]
 *
 *     GET/SET/DELETE: directly on the uppermost object.
 *
 *     NOTE: circular reference will trigger an exception on SET
 *
 */

import std

loc a = { val1: 10, val2: 20, val3: 30 }

print(a.val1)
print(a.val2)
print(a.val3)

a.proto = {
	val3: "old",
	fromp: "prototype"
}

print(a.val3)

a.val3 = "yeah"

print(a.proto.val3)

print(a.fromp)

a["proto"] = {}

print(a.fromp == none)

try a["proto"] = a
catch print("failed")

// op slots

numeric.proto["%"] = fn print("hi")

1 % 2

del numeric["proto"]["%"]

try 1 % 2 catch print("not defined")

numeric["prot" + "o"].% = fn print("hello")

1 % 2

numeric["pro" + "to"].-->> = fn print("arrow!")

1 -->> 2

numeric["pro" + "to"]["--" + ">"] = fn print("su~")

--> 1

ret

// -> "num: 10"
// -> "num: 20"
// -> "num: 30"
// -> "num: 30"
// -> "str: old"
// -> "str: prototype"
// -> "num: 1"
// -> "str: failed"
// -> "str: hi"
// -> "str: not defined"
// -> "str: hello"
// -> "str: arrow!"
// -> "str: su~"
