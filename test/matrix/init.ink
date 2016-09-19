import math

// assert v1.size() == v2.size()
loc dot_mul = fn v1, v2: {
	loc r = 0

	for [ loc a, loc b ] in v1.zip(v2):
		r = r + a * b

	r
}

loc rand_mat = fn n, m, min, max: {
	loc r = [ none ] * n
	loc i = 0

	while i < n: {
		r[i] = [ none ] * m
		loc j = 0

		while j < m: {
			r[i][j] = math.random(min, max)
			j = j + 1
		}

		i = i + 1
	}

	r
}
