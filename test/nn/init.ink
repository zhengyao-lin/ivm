import math
$import("../ulist")

loc.merge($import("../matrix"))

loc sigmoid = fn x: (1 / (1 + math.exp(-x)))

loc bpnn = fn arch, alpha, activate: { // arch = [ dim, [ hidden, dim ], dim ]
	alpha = alpha || 0.1
	activate = activate || sigmoid

	// print(activate(-1.060))

	loc hid = arch[1]
	loc dim = 1 + hid[0]

	loc params = [ none ] * dim // params[0] => weights of the inputs of the first hidden layer
	loc dims = [ arch[0] ] + [ hid[1] ] * hid[0] + [ arch[2] ]

	for loc i in range(dim): {
		// print(dims[i + 1] + " x " + (dims[i] + 1))
		params[i] = rand_mat(dims[i + 1], dims[i] + 1, -1, 1)
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

			for layer in range(layer, 0, -1): {
				loc d = [ 0 ] * node_count(layer - 1)

				// ignore the bias node
				for loc i in range(params[layer].size()): {
					loc psize = d.size() // count of nodes of the previous layer
					
					for loc j in range(psize): {
						loc tmp = params[layer][i][j] * prev_delta[i - 1]
						d[j] = d[j] + tmp
						total = total + math.abs(tmp)
					}
				}

				deltas.push(d)
				prev_delta = d
			}

			[ deltas.reverse(), total ]
		},

		update_params: fn os, errors, debug: {
			loc deriv = fn x: x * (1 - x)

			// loc err = 0
			// loc count = 0

			for loc i in range(params.size()): {
				// os[i] the outputs of the last layer
				// os[i + 1] the outputs of the current layer
				// params[i] the parameters from last layer to the current layer 


				for loc j in range(node_count(i) - 1): {
					loc e = errors[i][j + 1]
					loc dr = deriv(os[i + 1][j])
					loc p = params[i][j]

					for loc z in range(params[i][0].size()): {
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
					}
				}

				// delta = alpha * errors[] * derive(os[i + 1][j]) * os[i][j]
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
