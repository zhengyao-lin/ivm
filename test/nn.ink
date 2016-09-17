import math
import ulist

loc sigmoid = fn x: 1 / (1 + math.exp(-x))

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

loc transpose = fn mat: {
	loc i = 0
	loc rows = mat.size()
	loc cols = mat[0].size()
	loc r = []

	while i < cols: {
		r[i] = [ none ] * rows

		loc j = 0
		while j < rows: {
			r[i][j] = mat[j][i]
			j = j + 1
		}
		i = i + 1
	}

	r
}

loc bias = fn inp: [ 1 ] + inp

loc dot_mul = fn v1, v2: { // assert inps.size() == weights.size()
	loc r = 0

	for [ loc a, loc b ] in v1.zip(v2):
		r = r + a * b

	r
}

loc vec_sub = fn v1, v2: {
	loc r = []

	for [ loc a, loc b ] in v1.zip(v2):
		r.push(a - b)

	r
}

// v1 * transpose(v2) => a matrix
loc vec_mul = fn v1, v2: {
	loc r = []
	loc i = 0
	loc size1 = v1.size()
	loc size2 = v2.size()

	while i < size1: {
		loc j = 0
		r[i] = [ none ] * size2
		while j < size2: {
			r[i][j] = v1[i] * v2[j]
			j = j + 1
		}

		i = i + 1
	}

	r
}

loc vec_self_add = fn v1, v2: {
	loc i = 0
	loc size = v1.size()

	while i < size: {
		v1[i] = v1[i] + v2[i]
		i = i + 1
	}
}

loc mat_self_add = fn m1, m2: {
	for [ loc row1, loc row2 ] in m1.zip(m2):
		vec_self_add(row1, row2)
}

loc similar_fill = fn mat, fill: {
	fill = fill || 0
	loc r = []

	for loc col in mat:
		r.push([ fill ] * col.size())

	r
}

loc apply_node = dot_mul

loc apply_layer = fn inps, params: { // inps is 1-dim, params is a 2-dim matrix
	loc zs = []
	loc as = []

	for loc ws in params: {
		loc z = apply_node(inps, ws)
		zs.push(z)
		as.push(sigmoid(z))
	}

	[ zs, as ]
}

loc apply_net = fn inps, params: { // inps is 1-dim, params is a 3-dim matrix
	loc out = bias(inps)
	loc bout = out
	loc all_a = [ bout ]

	for loc layer in params: {
		[ loc z, out ] = apply_layer(bout, layer)
		bout = bias(out)
		all_a.push(bout)
	}

	[ out, all_a ]
}

loc error_layer = fn delta, params: { // get the delta for the previous layer, delta is 1-dim, params is 2-dim
	loc pt = transpose(params)

	// pt.print()

	loc r = []

	for loc node in pt: {
		// print("here")
		// delta.print()
		r.push(dot_mul(node, delta))
	}

	r
}

loc shape = fn mat: {
	print(mat.size() + " x " + mat[0].size())
}

loc bp = fn out, expect, params, all_a, big_d: {
	loc delta = bias(vec_sub(out, expect))
	loc r
	// loc big_d = []

	// init big_d all zeros
	// for loc layer in params:
	//	big_d.push(similar_fill(layer, 0))
	loc tmp = fn i: {
		loc a = big_d[i]
		loc b = vec_mul(delta.slice(1), all_a[i])

		// delta.slice(1).print()

		shape(a)
		shape(b)
		print("b")
		delta.print()
		b.print()
		all_a[i].print()
		// print("delta")
		// delta.print()

		mat_self_add(a, b)
	}

	loc i = params.size() - 1

	tmp(i)

	while i > 0: {
		delta = delta.slice(1)
		delta = error_layer(delta, params[i])

		i = i - 1

		tmp(i)
	}
	// mat_self_add(big_d[i], vec_mul(delta, all_a[i - 1]))
}

loc init_nn = fn arch: { // arch = [ dim, [ hidden, dim ], dim ]
	loc hid = arch[1]
	loc dim = 1 + hid[0]

	loc params = [ none ] * dim // params[0] => weights of the inputs of the first hidden layer
	loc dims = [ arch[0] ] + [ hid[1] ] * hid[0] + [ arch[2] ]

	loc i = 0

	while i < dim: {
		print(dims[i + 1] + " x " + (dims[i] + 1))
		params[i] = rand_mat(dims[i + 1], dims[i] + 1, -1, 1)
		i = i + 1
	}

	[ params, dims ]
}

loc gen_d = fn params: {
	loc big_d = []

	// init big_d all zeros
	for loc layer in params:
		big_d.push(similar_fill(layer, 0))

	big_d
}
ret
/*
[ params, dims ] = init_nn([ 4, [ 2, 5 ], 2 ])

params[0].print()
print(apply_node([ 1, 2, 3 ], [ 0.1, 0.2, 0.3 ]))
apply_layer([ 1, 2, 3, 4 ], params[0]).print()
apply_net([ 0, 1, 0, 1 ], params).print()

loc m = rand_mat(3, 4)

m.print()
transpose(m).print()

vec_mul([ 1, 2, 3 ], [ 2, 3 ]).print()

similar_fill([ [ 1, 2, 3 ], [ 2, 4 ] ]).print()
*/

print(sigmoid(0.2))

[ params, dims ] = init_nn([ 1, [ 1, 2 ], 1 ])

loc big_d = gen_d(params)

print("params")
for loc p in params:
	p.print()
print("params")

[ out, all_a ] = apply_net([ 1 ], params)

all_a.print()

bp(out, [ 0 ], params, all_a, big_d)

big_d.print()

ret

print("################## bp #################")

alpha = 0.2
time = 0

[ params, dims ] = init_nn([ 1, [ 3, 5 ], 1 ])

// > 15?
train_ex = []

loc gen_train = fn: {
	loc i = 0

	while i < 100: {
		loc num = math.random(0, 30)
		train_ex.push([ [ num ], [ num > 15 ] ])
		i = i + 1
	}
}

gen_train()

while time < 100: {
	loc big_d = gen_d(params)

	for [ loc inp, loc expect ] in train_ex: {
		[ out, all_a ] = apply_net(inp, params)
		bp(out, expect, params, all_a, big_d)
	}

	// regression
	loc i = 0
	loc err = 0

	for loc layer in params: {
		loc j = 0
		for loc node in layer: {
			loc k = 0
			loc size = node.size()
			while k < size: {
				// print(i + ", " + j + ", " + k)
				err = err + big_d[i][j][k]
				node[k] = node[k] - alpha / train_ex.size() * big_d[i][j][k]
				k = k + 1
			}
			j = j + 1
		}
		i = i + 1
	}

	print(err)

	time = time + 1
}

print("end training")

loc i = 0
loc wrong = 0
loc test_t = 10000

while i < test_t: {
	loc num = math.random(0, 30)
	[ out, all_a ] = apply_net([ num ], params)

	if out[0] > 0.5 != num > 15: {
		print(num + " -> " + out[0])
		wrong = wrong + 1
	}

	i = i + 1
}

print(wrong / test_t * 100 + "%")
