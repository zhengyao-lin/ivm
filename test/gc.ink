import std

loc M = 1000000
loc B = 2 * 5 * 4 * 4 * 4 + 1
loc A = 100001

loc new_obj = fn h, seed: {
	loc i = seed

	for loc j in range(10000): {
		h.push({})
	}

	for loc j in range(10000): {
		i = ((A * i) + B) % M
		h[i % h.size()] = none
	}
}

loc heap = []

for loc k in range(100): {
	new_obj(heap, k)
}
