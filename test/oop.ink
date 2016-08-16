a = 10
a.+ = fn b: base - b
a.* = "hi"

print(a + 5)
try: a * 5
catch err: print(err.msg)

res = 1

a.<= = fn b: res
a.>= = fn b: a <= b
a.< = fn b: a >= b
a.> = fn b: a < b

try: 10 + ""

if a > 11: print("yes!")
else: print("no!")

res = 0

if a > 11: print("no!")
else: print("yes!")

// -> "num: 5.000"
// -> "str: unable to invoke object of type <string>"
// -> "str: yes!"
// -> "str: yes!"
