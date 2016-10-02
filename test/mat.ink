import math

print(1.23.round())
print(1.5000001.round())
print(1.4999999.round())
print(1.23.ceil())
print(1.9999.floor())

print(math.cos(math.rad(30)))
print(math.sin(math.rad(30)))
print(math.tan(math.rad(30)))

print(math.deg(math.acos(math.cos(math.rad(3)))) - 3 <= 0.0000001)
print(math.deg(math.asin(math.sin(math.rad(2)))) - 2 <= 0.0000001)
print(math.deg(math.atan(math.tan(math.rad(1)))) - 1 <= 0.0000001)

print(math.log(math.exp(20)))
print(math.exp(math.log(21)))
print(math.log10(math.pow(10, 11)))

print(math.sqrt(math.pow(11, 2)))

// exceptions
try: print(math.sqrt(-0.0001))
catch: print("yes")

print((1 / 0).isinf())
print(!(1 / 0).isneginf())
print((1 / 0).isposinf())

print((1 / -0).isneginf())
print((-1 / 0).isneginf())

print(math.nan.isnan())
print(math.inf.isinf())
print(math.inf.isposinf())

print(math.nan)

ret

// -> "num: 1"
// -> "num: 2"
// -> "num: 1"
// -> "num: 2"
// -> "num: 1"
// -> "num: 0.866.*"
// -> "num: 0.499.*"
// -> "num: 0.577.*"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 20"
// -> "num: 21"
// -> "num: 11"

// -> "num: 11"
// -> "str: yes"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: 1"
// -> "num: -nan"
