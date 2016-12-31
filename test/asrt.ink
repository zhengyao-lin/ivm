import std

print(assert 10)

try: assert false
catch: print("yeah")

ret

// -> "num: 10"
// -> "str: yeah"
