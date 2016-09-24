import ulist
import math
import nn

loc norm = fn outs: {
	if typeof(outs) == "list": {
		loc r = []

		for loc o in outs:
			r.push(norm(o))

		r
	} else: outs > 0.5
}

loc test_not = fn: {
	loc net = nn.bpnn([ 1, [ 1, 3 ], 1 ], 0.2)
	loc i = 0

	while i < 1000: {
		net.train([ 0 ], [ 1 ])
		loc err =
		net.train([ 1 ], [ 0 ])

		print("error: " + err)

		i = i + 1
	}

	norm(net.predict([ 1 ])).print()
	norm(net.predict([ 0 ])).print()
}

loc test_gate = fn f: {
	loc alpha = 0.2
	loc net = nn.bpnn([ 2, [ 1, 3 ], 1 ], alpha)
	loc i = 0
	loc err = 10000
	loc thres = 100

	loc tests = [
		[ [ 0, 0 ], [ f(0, 0) ] ],
		[ [ 0, 1 ], [ f(0, 1) ] ],
		[ [ 1, 0 ], [ f(1, 0) ] ],
		[ [ 1, 1 ], [ f(1, 1) ] ]
	]

	while err > 0.02: {
		for loc t in tests: {
			loc err = net.train(t[0], t[1])
		}

		if i > thres: {
			// print("raise alpha")
			net.set_alpha(alpha = alpha * 1.5)
			thres = thres * 1.2
		}
		
		// print("error: " + err)
		
		i = i + 1
	}

	[
		norm(net.predict([ 0, 0 ])[-1]),
		norm(net.predict([ 0, 1 ])[-1]),
		norm(net.predict([ 1, 0 ])[-1]),
		norm(net.predict([ 1, 1 ])[-1])
	].print()
}

loc test_bin2bool = fn ev, jud: {
	loc min = 0
	loc max = 100

	loc dnorm = fn a: {
		(a - min) / (max - min)
	}

	loc denorm = fn a: {
		a * (max - min) + min
	}

	loc alpha = 0.2
	loc net = nn.bpnn([ 2, [ 1, 2 ], 1 ], alpha)
	loc tests = []
	loc err = 10000
	loc thres = 100
	loc tcount = 300
	loc max_iter = 200

	for loc i in range(tcount): {
		loc a = math.random(min, max)
		loc b = math.random(min, max)
		tests.push([ [ dnorm(a), dnorm(b) ], [ jud(ev(a, b)) ] ])
	}

	loc i = 0
	loc ave_err = 1000

	print("training")

	while ave_err > 0.4 && i < max_iter: {
		loc ave_err = 0

		for loc t in tests: {
			ave_err = ave_err + net.train(t[0], t[1])
		}

		ave_err = ave_err / tcount

		// print("average error: " + ave_err + ", alpha: " + alpha + ", iter: " + i)

		alpha = math.random()

		// if i > thres: {
			// print("raise alpha")
		// 	net.set_alpha(alpha = alpha * 1.3)
		// 	thres = thres * 1.5
		// }
		
		i = i + 1
	}

	// net.params.print()

	tests = []

	for loc i in range(tcount): {
		loc a = math.random(min, max)
		loc b = math.random(min, max)
		tests.push([ [ dnorm(a), dnorm(b) ], [ jud(ev(a, b)) ] ])
	}

	loc wrong = 0

	for loc t in tests: {
		loc res = net.predict(t[0])[-1][0]

		// print(denorm(t[0][0]) + ", " + denorm(t[0][1]) + " -> " + res + ", real: " + t[1][0])

		if norm(res) != t[1][0]: {
			loc a = denorm(t[0][0])
			loc b = denorm(t[0][1])

			wrong = wrong + 1
			
			// print("wrong: " + a + ", " + b +
			// 	  " -> " + ev(a, b) +
			// 	  " -> predict: " + res + ", real: " + t[1][0])
		}
	}

	loc perc = wrong / tcount * 100

	if perc < 10:
		print("predict success")

	print("error rate: " + perc + "%")
}

// test_not()
test_gate to | a, b | a | b
test_bin2bool((fn a, b: math.abs(math.abs(a) - math.abs(b))), (fn a: a < 50))

ret

// -> "str: \\[ \\[ 0 \\], \\[ 1 \\], \\[ 1 \\], \\[ 1 \\] \\]"
// -> "str: training"
// -> "str: predict success"
// -> "str: error rate: .*%"
