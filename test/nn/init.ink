import math
`import`("../ulist")

loc.merge(`import`("../matrix"))

loc sigmoid = fn x: (1 / (1 + math.exp(-x)))

loc bpnn = fn arch, alpha, activate: { // arch = [ dim, [ hidden, dim ], dim ]
	alpha = alpha || 0.1
	activate = activate || sigmoid

	// print(activate(-1.060))

	loc hid = arch[1]
	loc dim = 1 + hid[0]

	loc params = [ none ] * dim // params[0] => weights of the inputs of the first hidden layer
	loc dims = [ arch[0] ] + [ hid[1] ] * hid[0] + [ arch[2] ]

	loc i = 0

	while i < dim: {
		// print(dims[i + 1] + " x " + (dims[i] + 1))
		params[i] = rand_mat(dims[i + 1], dims[i] + 1, -1, 1)
		i = i + 1
	}

	trace("params: " + params.to_str())

	loc bias = fn orig: [ 1 ] + orig
	loc vec_min = fn v1, v2: {
		loc r = []

		for [ loc a, loc b ] in v1.zip(v2):
			r.push(a - b)

		r
	}

	loc node_count = fn layer_num: {
		// include the bias node
		params[layer_num].size() + 1
	}

	{
		params: params,

		// forward propagation
		predict: fn inputs: {
			loc os = [ inputs ] // all outputs

			for loc layer in params: {
				loc cur_os = [] // outputs

				for loc node in layer: {
					loc o = dot_mul(bias(inputs), node)
					// print(o)
					cur_os.push(activate(o))
				}

				// print(cur_os.to_str())

				inputs = cur_os
				os.push(cur_os)
			}

			os
		},

		get_error: fn os, target: {
			loc deltas = []
			loc layer = params.size() - 1
			loc prev_delta = [ 0 ] + vec_min(target, os[layer + 1])
			loc total = 0

			deltas.push(prev_delta)

			while layer > 0: {
				loc i = 0 // ignore the bias node
				loc size = params[layer].size()
				
				loc d = [ 0 ] * node_count(layer - 1)

				while i < size: {
					loc j = 0
					loc psize = d.size() // count of nodes of the previous layer
					
					while j < psize: {
						loc tmp = params[layer][i][j] * prev_delta[i - 1]
						d[j] = d[j] + tmp
						total = total + math.abs(tmp)
						j = j + 1
					}

					i = i + 1
				}

				deltas.push(d)
				prev_delta = d

				layer = layer - 1
			}

			[ deltas.reverse(), total ]
		},

		update_params: fn os, errors, debug: {
			loc i = 0
			loc lcount = params.size()

			loc deriv = fn x: x * (1 - x)

			// loc err = 0
			// loc count = 0

			while i < lcount: {
				// os[i] the outputs of the last layer
				// os[i + 1] the outputs of the current layer
				// params[i] the parameters from last layer to the current layer 

				loc j = 0
				loc ncount = node_count(i) - 1

				while j < ncount: {
					loc z = 0
					loc pnsize = params[i][0].size()

					loc e = errors[i][j + 1]
					loc dr = deriv(os[i + 1][j])
					loc p = params[i][j]

					while z < pnsize: {
						// trace("layer: " + (i + 1) + ", node: " + j + ", from: " + z)
						// trace("   error: " + errors[i][j + 1])
						// print(j)
						// trace("   self output: " + os[i + 1][j])
						// trace("   from output: " + os[i][z - 1])
						// trace("   origin weight: " + params[i][j][z])

						if z == 0: {
							// trace("   (weight from bias node)")
							loc delta = e * dr // * 1
						} else:
							loc delta = e * dr * os[i][z - 1]

						// trace("   -> delta: " + delta)

						// err = err + math.abs(delta)
						// count = count + 1

						p[z] = p[z] + alpha * delta

						z = z + 1
					}

					j = j + 1
				}

				// delta = alpha * errors[] * derive(os[i + 1][j]) * os[i][j]

				i = i + 1
			}

			// err
		},

		train: fn inputs, outputs: {
			loc os = base.predict(inputs)
			[ loc errors, loc total ] = base.get_error(os, outputs)
			base.update_params(os, errors)

			total
		},

		set_alpha: fn n: {
			alpha = n
		}
	}
}

loc trace = if 0: print else: fn: none
