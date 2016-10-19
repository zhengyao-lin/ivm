(fn: {
	lp = [].proto

	lp.each = fn f: {
		for loc e in base: f(e)
	}

	lp.map = lp.apply = fn f: {
		loc r = []
		for loc e in base:
			r.push(f(e))
		r
	}

	lp.filter = fn f: {
		loc r = []

		for loc e in base:
			if f(e):
				r.push(e)

		r
	}

	lp.reduce = fn f, init: {
		loc size = base.size()

		if size <= 1: ret base[0]

		loc i = 1

		if init != none:
			loc val = f(init, base[0])
		else:
			loc val = base[0]

		while i < size: {
			val = f(val, base[i])
			i = i + 1
		}

		val
	}

	lp.to_str = fn: {
		loc r = []

		for loc e in base: {
			loc t = typeof(e)
			if t == "numeric" || t == "string":
				r.push(e)
			elif t == "list":
				r.push(e.to_str())
			else:
				r.push("<" + t + ">")
		}

		if r.size(): "[ " + r.cat(", ") + " ]"
		else: "[]"
	}

	lp.print = fn: {
		print(base.to_str())
	}

	lp.zip = fn arr: {
		loc r = []
		loc s1 = base.size()
		loc s2 = arr.size()
		loc i = 0

		while i < s1 && i < s2: {
			r.push([base[i], arr[i]])
			i = i + 1
		}

		r
	}

	lp.uniq = fn: {
		loc r = []

		for loc e1 in base: {
			loc set = 0
			for loc e2 in r:
				if e1 == e2: {
					set = 1
					break
				}
			if !set:
				r.push(e1)
		}

		r
	}

	lp.reverse = fn: {
		loc r = base.clone()
		loc size = r.size()
		loc e = size / 2
		loc i = 0
		
		while i < e: {
			loc tmp = r[i]
			r[i] = r[size - i - 1]
			r[size - i - 1] = tmp
			i = i + 1
		}

		r
	}

	lp.sum = fn: {
		if !base.size(): none
		else: {
			loc s = base[0]
			for loc e in base.slice(1):
				s = s + e
			s
		}
	}

	lp.cat = fn mid: {
		if !base.size(): none
		else: {
			mid = mid || ""
			loc s = base[0]
			for loc e in base.slice(1):
				s = s + mid + e
			s
		}
	}

	lp.fill = fn n, v = 0: {
		loc r = base.clone()
		loc s = r.size()

		if n < s:
			r.slice(, n)
		else: {
			for loc i in range(n - s): r.push(v)
			r
		}
	}
})()
