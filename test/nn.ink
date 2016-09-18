import math
import ulist

loc activate = fn x: 1 / (1 + math.exp(-x))

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

loc num_sub_vec = fn n, v: {
	loc r = []

	for loc e in v:
		r.push(n - e)

	r
}

loc vec_mul_each = fn v1, v2: {
	loc r = []

	for [ loc e1, loc e2 ] in v1.zip(v2):
		r.push(e1 * e2)

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
		as.push(activate(z))
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

loc error_out = fn as, expect: {
	// error = (as - expect) * as * (1 - as)
	ret vec_sub(expect, as)
	// vec_mul_each(vec_sub(as, expect), vec_mul_each(as, num_sub_vec(1, as)))
}

loc error_layer = fn as, delta, params: {
	// get the delta for the previous layer,
	// delta is 1-dim,
	// params is 2-dim(the weights between the previous layer and the current layer)
	loc pt = transpose(params)

	loc r = []

	for loc node in pt: {
		r.push(dot_mul(node, delta))
	}

	ret r

	r = vec_mul_each(r, vec_mul_each(as, num_sub_vec(1, as)))
	// print("r: " + as.to_str())

	r
}

loc shape = fn mat: {
	print(mat.size() + " x " + mat[0].size())
}

loc get_deltas = fn out, expect, params, all_a: {
	loc delta = error_out(out, expect)

	loc tmp = fn i, trunc: {
		loc a = big_d[i]

		// print("delta: " + delta.to_str())

		loc b = vec_mul(delta, all_a[i])

		// delta.slice(1).print()

		// shape(a)
		// shape(b)
		/*
		print("b")
		delta.print()
		b.print()
		all_a[i].print()
		*/
		// print("delta")
		// delta.print()
		// print("as: " + all_a[i].to_str())
		// print("grads: " + b.to_str())

		mat_self_add(a, b)
	}

	loc i = params.size() - 1
	loc r = []

	// tmp(i, 0)

	r[i] = delta

	while i > 0: {
		delta = error_layer(all_a[i], delta, params[i])

		i = i - 1

		delta = delta.slice(1)
		r[i] = delta

		// tmp(i, 1)
	}

	r
	// mat_self_add(big_d[i], vec_mul(delta, all_a[i - 1]))
}

loc init_nn = fn arch: { // arch = [ dim, [ hidden, dim ], dim ]
	loc hid = arch[1]
	loc dim = 1 + hid[0]

	loc params = [ none ] * dim // params[0] => weights of the inputs of the first hidden layer
	loc dims = [ arch[0] ] + [ hid[1] ] * hid[0] + [ arch[2] ]

	loc i = 0

	while i < dim: {
		// print(dims[i + 1] + " x " + (dims[i] + 1))
		params[i] = rand_mat(dims[i + 1], dims[i] + 1, -10, 10)
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

loc normalize = fn train_ex, idx: {
	loc min = [ 0 ] * train_ex[0][idx].size()
	loc max = train_ex[0][idx].clone()

	for loc ex in train_ex: {
		loc i = 0
		loc size = ex[idx].size()

		while i < size: {
			if ex[idx][i] < min[i]:
				min[i] = ex[idx][i]

			if ex[idx][i] > max[i]:
				max[i] = ex[idx][i]

			i = i + 1
		}
	}

	// min.print()
	// max.print()

	loc r = []

	for loc ex in train_ex: {
		loc i = 0
		loc size = ex[idx].size()

		while i < size: {
			ex[idx][i] = (ex[idx][i] - min[i]) / (max[i] - min[i])
			i = i + 1
		}
	}
}

loc deriv_activate = fn n: {
	n = activate(n)
	n * (1 - n)
}

loc update_params = fn all_a, params, deltas, alpha: {
	loc lay = 0
	loc lcount = params.size()
	loc delta_sum = 0

	while lay < lcount: {
		loc i = 0 // the current node in the current layer
		loc size = params[lay].size()

		while i < size: {
			loc j = 0
			loc pn_size = params[lay][i].size() // previous node count

			while j < pn_size: {
				print(lay + ": " + j + " -> " + i)
				print("   delta: " + deltas[lay][i])
				print("   deriv: " + deriv_activate(all_a[lay + 1][i + 1]))
				print("   self out: " + all_a[lay + 1][i + 1])
				print("   output: " + all_a[lay][j])

				loc delta = alpha * deltas[lay][i] * deriv_activate(all_a[lay + 1][i + 1]) * all_a[lay][j]

				delta_sum = delta_sum + math.abs(delta)

				params[lay][i][j] = params[lay][i][j] + delta

				j = j + 1
			}

			i = i + 1
		}

		lay = lay + 1
	}

	//print("delta sum: " + delta_sum)
}

ret

loc test1 = fn: {
	loc train_time = 1
	loc i = 0
	loc train_ex = [ // xor
		[ [ 0, 0 ], [ 1 ] ],
		[ [ 1, 0 ], [ 0 ] ],
		[ [ 0, 1 ], [ 0 ] ],
		[ [ 1, 1 ], [ 0 ] ]
	]

	[ params, dims ] = init_nn([ 2, [ 2, 2 ], 1 ])

	while i < train_time: {
		for [ loc inp, loc expect ] in train_ex: {
			[ out, all_a ] = apply_net(inp, params)
			deltas = get_deltas(out, expect, params, all_a)
			update_params(all_a, params, deltas, 0.1)

			ret
		}
		i = i + 1
	}

	loc norm = fn out: {
		loc r = []

		for loc o in out:
			r.push(o > 0.5)

		r
	}

	[ out, all_a ] = apply_net([ 1, 1 ], params)
	norm(out).print()

	[ out, all_a ] = apply_net([ 1, 0 ], params)
	norm(out).print()

	[ out, all_a ] = apply_net([ 0, 1 ], params)
	norm(out).print()

	[ out, all_a ] = apply_net([ 0, 0 ], params)
	norm(out).print()
}

test1()

ret

loc test1 = fn: {
	loc train_time = 1000
	loc i = 0
	loc train_ex = [
		[ [ 0, 0, 0 ], [ 1, 1, 1 ] ],
		[ [ 1, 0, 0 ], [ 0, 1, 1 ] ],
		[ [ 0, 1, 0 ], [ 1, 0, 1 ] ],
		[ [ 0, 0, 1 ], [ 1, 1, 0 ] ],
		[ [ 1, 1, 0 ], [ 0, 0, 1 ] ],
		[ [ 0, 1, 1 ], [ 1, 0, 0 ] ],
		[ [ 1, 0, 1 ], [ 0, 1, 0 ] ],
		[ [ 1, 1, 1 ], [ 0, 0, 0 ] ]
	]

	[ params, dims ] = init_nn([ 3, [ 1, 4 ], 3 ])

	while i < train_time: {
		for [ loc inp, loc expect ] in train_ex: {
			[ out, all_a ] = apply_net(inp, params)
			deltas = get_deltas(out, expect, params, all_a)
			update_params(all_a, params, deltas, 0.1)
		}
		i = i + 1
	}

	loc norm = fn out: {
		loc r = []

		for loc o in out:
			r.push(o > 0.5)

		r
	}

	[ out, all_a ] = apply_net([ 1, 1, 0 ], params)
	norm(out).print()

	[ out, all_a ] = apply_net([ 1, 0, 0 ], params)
	norm(out).print()

	[ out, all_a ] = apply_net([ 0, 0, 0 ], params)
	norm(out).print()
}

// test1()
// ret

loc alpha = 0.1
loc train_time = 100

[ params, dims ] = init_nn([ 1, [ 2, 3 ], 1 ])

train_ex = []
test_ex = []

tmp = (fn: {
	loc i = 0
	loc r = []

	while i < 1000: {
		loc num = math.random(0, 100)
		r.push([ [ num ], [ num * num ] ])

		i = i + 1
	}

	r
})()

normalize(tmp, 0)
normalize(tmp, 1)

train_ex = tmp.slice(0, 50)
test_ex = tmp.slice(50)

loc time = 0

while time < train_time: {
	for [ loc inp, loc expect ] in train_ex: {
		// print("#####################")
		// each train data

		// print("params: " + params.to_str())

		// print("big d: " + big_d.to_str())
		// print("input: " + inp.to_str())
		// print("expect: " + expect.to_str())

		[ out, all_a ] = apply_net(inp, params)

		// print("output: " + out.to_str())

		// print("all a: " + all_a.to_str())

		deltas = get_deltas(out, expect, params, all_a, big_d)

		// print("deltas: " + deltas.to_str())

		// print("params prev: " + params.to_str())

		update_params(all_a, params, deltas, alpha)

		// print("params after: " + params.to_str())

		//ret
	}

	//ret

	time = time + 1
}

// for loc lay in params:
// 	print(lay.to_str())

// ret

/*
print("end training")

[ out, all_a ] = apply_net([ 1 ], params)
out.print()

[ out, all_a ] = apply_net([ 0 ], params)
out.print()

ret
*/

print("end training")

loc i = 0
loc wrong = 0
loc test_t = test_ex.size()

for loc t in test_ex: {
	[ out, all_a ] = apply_net(t[0], params)

	loc delta = math.abs(out[0] - t[1][0])

	// print(delta)

	if delta > 0.3: {
		print("delta: " + delta)
		// print("correct: " + t[1][0] + ", predict: " + out[0])
		wrong = wrong + 1
	}

	i = i + 1
}

print("error rate: " + wrong / test_t * 100 + "%")

ret
