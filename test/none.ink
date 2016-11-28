import std

loc a = none

try: print(a.b)
catch: print("failed to get slot of none")

try: a.b = 10
catch: print("failed to set slot of none")

try: print(a.proto)
catch: print("failed to get proto of none")

try: a.proto = 20
catch: print("failed to set proto of none")

try: print(a.+)
catch: print("failed to get add op of none")

try: a.+ = 1
catch: print("failed to set add op of none")

print(a == none)
print(not_defined == a)
print(!a)

print(a == 1)
print(a == "")

loc f = fn: {
	loc = none
	loc msg = "yes"
	print(msg)
}

f()

// -> "str: failed to get slot of none"
// -> "str: failed to set slot of none"
// -> "str: failed to get proto of none"
// -> "str: failed to set proto of none"
// -> "str: failed to get add op of none"
// -> "str: failed to set add op of none"
// 
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 0"
// -> "num: 0"
// 
// -> "str: yes"
