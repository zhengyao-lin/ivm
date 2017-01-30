import std
import math
import ulist

loc n = 1000
loc isprime = [ 1 ] * (n + 1)

isprime[0] = false
isprime[1] = false

loc seive = fn: {
	for loc i in range(2, math.sqrt(n) + 1): {
		if isprime[i]:
			for loc j in range(2, n / i + 1):
				isprime[i * j] = false
	}

	ret
}

seive()
print(isprime.sum())

loc tot = 0

[
	(tot += 1, i)
	for loc i in range(2, n)
	if !isprime[i] && i % 2 && i % 3 && i % 5
] // .print()

print(tot)

// -> "num: 168"
// -> "num: 100"
