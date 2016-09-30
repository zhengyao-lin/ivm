try: "abcdefghi".ord(20)
catch: print("exception")

print("abcdefghi".ord())

try: print("".ord())
catch: print("yes")

print("".len())
print("22".len())

print("y".ord().char())

print(typeof("0"[10]))

ret

// -> "str: exception"
// -> "num: 97"
// -> "str: yes"
// -> "num: 0"
// -> "num: 2"
// -> "str: y"
// -> "str: none"
