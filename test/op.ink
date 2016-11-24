print(1 << 2)		// -> "num: 4"
print(1024 >> 2)	// -> "num: 256"
print(-1 >> 2)		// -> "num: -1"
print(-1 >>> 1)		// -> "num: 9223372036854776000"

try: print(1 % 0)
catch: print("mod zero")

try: print(1 / 0)
catch: print("div zero")

ret

// -> "str: mod zero"
// -> "str: div zero"
