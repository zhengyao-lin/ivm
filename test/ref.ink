
del b

loc a = ref b
deref a = 10
print(b) // -> "num: 10"

loc a = ref b[0]

try: print(deref a)
catch: print("failed to get deref a as b[0]")
// -> "str: failed to get deref a as b\\[0\\]"

try: deref a = 10
catch: print("failed to set deref a as b[0]")
// -> "str: failed to set deref a as b\\[0\\]"

loc f = fn args..., out: {
	loc i = 0
	loc size = args.size()

	while i < size: {
		(deref out)["arg " + (i + 1)] = args[i]
		i = i + 1
	}
}

loc outv = {}
f("it's arg 1", "arg2", "three", ref outv)

print(outv["arg 1"]) // -> "str: it's arg 1"
print(outv["arg 2"]) // -> "str: arg2"
print(outv["arg 3"]) // -> "str: three"
print(outv["arg 4"] == none) // -> "num: 1"
