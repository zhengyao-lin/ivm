try: "abcdefghi".char(20)
catch: print("exception")

print("abcdefghi".char())

try: print("".char())
catch: print("yes")

print("".len())
print("22".len())

ret

// -> "str: exception"
// -> "num: 97"
// -> "str: yes"
// -> "num: 0"
// -> "num: 2"
