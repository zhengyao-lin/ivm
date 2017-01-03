import std
import ulist

a = [0, 0, 0]

print(1 && (try {
	try a[a[a[a[a[a[a[a[{} + {}]]]]]]]] catch
	try a[{} + {}] catch none final {
		"oops"; {} + {}
	}
} catch none final "yep"))

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
