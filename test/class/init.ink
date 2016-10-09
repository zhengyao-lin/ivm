
loc class = fn mixins..., init: {
	loc r = fn: {
		loc n = { proto: r.proto }
		init(n)
		ret n
	}

	r.core = init
	if mixins.size(): {
		loc p = r.proto = mixins[0]()
		for loc mixin in mixins.slice(1):
			mixin.core(p)
	}

	ret r
}
