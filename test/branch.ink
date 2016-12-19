import std

print(1 ? 0 : 1)
print(1 && 0 || 1)

print(0 ? fn: 1 ? "no" : "nop" : "yes")

!a ? a = 1 ? "yes" : "no" : "no!!"
print(a)

a = 1 && 1 ? a ? "hey" : "no" : 0

print(a)

loc a = 10

if a == 1: print("what")
elif a == 101: print("1")
elif a == 10: print("yes")
elif a == 10: print("3")
elif a == 10: print("no")

ret

// -> "num: 0"
// -> "num: 1"
// -> "str: yes"
// -> "str: yes"
// -> "str: hey"

// -> "str: yes"
