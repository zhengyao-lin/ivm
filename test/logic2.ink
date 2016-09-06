[].proto.each = fn f:
	for loc e in base:
		f(e)

[].proto.apply = fn f: (
	loc r = [],
	for loc e in base: r.push(f(e)),
	r
)

[].proto.sum = fn: {
	if !base.size(): none
	else: {
		loc s = base[0]
		for loc e in base.slice(1):
			s = s + e
		s
	}
}

[].proto.cat = fn mid: {
	if !base.size(): none
	else: {
		mid = mid || ""
		loc s = base[0]
		for loc e in base.slice(1):
			s = s + mid + e
		s
	}
}

a = [0, 0, 0]

print(1 && (try: (try: a[a[a[a[a[a[a[a[{} + {}]]]]]]]], try: a[{} + {}] catch: none final: "oops", {} + {}) final: "yep"))
print(none || a.apply(fn a: a * 2).sum() && 1)
print(1 && 1 && 1 && 1 && 1 && 1 && 1 && 1 && 1 && 1 && 0 || "yes")

print(["a", "b", "c"].cat(", "))

print([1, 2, 3, 4].slice(1).size())

ret

// -> "str: yep"
// -> "num: 0"
// -> "str: yes"
// -> "str: a, b, c"
// -> "num: 3"
