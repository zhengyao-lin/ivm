import std

print("\\a") // -> "str: \\\\a"
print("\xe4\xbd\xa0\xe5\xa5\xbd" == "你好") // -> "num: 1"

print("\e4") // -> "str: e4"
print("\d33\x21") // -> "str: !!"

print("\xe4\xbd\xa0\xe5\xa5\xbd".len()) // -> "num: 2"

print("\12322") // -> "str: S22"
print("\o12322") // -> "str: S22"
print("\x5922") // -> "str: Y22"
print("\d08922") // -> "str: Y22"

print("\u4f60\u597d \u10398 \u10380 \u10381" ==
	  "\xe4\xbd\xa0\xe5\xa5\xbd \xf0\x90\x8e\x98 \xf0\x90\x8e\x80 \xf0\x90\x8e\x81")
// -> "num: 1"

print("\uA5F0 \uA604 \u0d02" ==
	  "\xea\x97\xb0 \xea\x98\x84 \xe0\xb4\x82")
// -> "num: 1"

// -> "num: 804"
// -> "num: 5"
// -> "num: 527"
// -> "num: 527"
print(0x324)
print(0b101)
print(0o1017)
print(0O1017)

ret
