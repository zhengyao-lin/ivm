a = 10
a@+ = fn b: base - b
a@* = "hi"

print(a + 5)
try: a * 5
catch err: print(err.msg)

// -> "num: 5.000"
// -> "str: unable to invoke object of type <string>"
