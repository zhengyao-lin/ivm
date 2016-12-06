loc class = fn *mixin, init: {
	loc p = mixin.size() && mixin[0] || object.proto

	loc r = fn *args: {
		loc n = { proto: p }
		init(n, *args)
		ret n
	}

	r.proto = p

	ret r
}
