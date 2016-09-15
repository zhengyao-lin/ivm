import math
import ulist

loc strToList = fn str: {
	loc i = 0
	loc len = str.len()
	loc r = []

	while i < len: {
		r.push(str.ord(i))
		i = i + 1
	}

	r
}

loc listToStr = fn list: {
	loc r = ""

	for loc c in list:
		r = r + c.char()

	r
}

target = strToList("hello, world")
dna_size = target.size()

init_count = 100
live_rate = 0.7
mutate_prob = 0.1
min_p = 2
max_count = 100

dna_min = 32
dna_max = 123

species = []

loc randArray = fn n, min, max: {
	loc r = []
	loc i = 0

	while i < n: {
		r.push(math.random(min, max))
		i = i + 1
	}

	ret r
}

loc ga_init = fn: {
	loc i = 0

	species = []
	while i < init_count: {
		species.push(randArray(dna_size, dna_min, dna_max))
		i = i + 1
	}
}

loc ga_fitness = fn dna: {
	loc cor_count = 0

	for [a, b] in dna.zip(target): {
		if a.floor() == b: cor_count = cor_count + 1
	}

	math.pow(2, cor_count)
}

loc ga_getSurvProb = fn: {
	loc fits = []

	for loc s in species:
		fits.push(ga_fitness(s))

	loc prob = []
	loc sum = fits.sum()

	for loc f in fits:
		prob.push(f / sum)

	prob
}

loc ga_choose = fn lprob: {
	loc choose = fn: {
		loc pos = math.random() // 0 - 1
		loc cur = 0
		loc i = 0
		loc size = species.size()

		// print(pos)

		while i < size: {
			cur = cur + lprob[i]
			if cur > pos:
				ret i
			// print(cur)
			i = i + 1
		}

		raise { msg: "impossible" }
	}

	loc i = 0
	loc chosen = []
	loc live_count = live_rate * species.size()

	if live_count > max_count:
		live_count = max_count

	while i < live_count: {
		chosen.push(choose(lprob))
		i = i + 1
	}

	loc orig = species
	species = []

	for loc i in chosen:
		species.push(orig[i])

	chosen
}

loc ga_reprod = fn: {
	loc pair = fn: {
		loc i = 0
		loc r = []
		loc size = species.size()

		while i < size: {
			if i + 1 < size: {
				r.push([ species[i], species[i + 1] ])
				i = i + 2
			} else: break
		}

		r
	}

	loc pairs = pair()

	for [loc a, loc b] in pairs: {
		loc res = []
		for [ loc i, loc j ] in a.zip(b): {
			loc p = math.random()
			if p < mutate_prob:
				res.push(math.random(dna_min, dna_max))
			elif p < 0.5:
				res.push(i)
			else:
				res.push(j)
		}
		species.push(res)
	}
}

loc ga_best = fn: {
	loc max_f = 0
	loc max_s = none

	for loc s in species: {
		loc f = ga_fitness(s)
		// print(f)

		if f > max_f: {
			max_f = f
			max_s = s
		}
	}

	[ max_s, max_f ]
}

ga_init()

// for loc s in species:
//	print(ga_fitness(s))

loc i = 0

while i < 100: {
	ga_choose(ga_getSurvProb())
	ga_reprod()
	i = i + 1
}

[ loc s, loc fitness ] = ga_best()
print(listToStr(s) + " -> " + fitness)

ret

// -> "str: .*"
