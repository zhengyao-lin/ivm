print(1 ? 0 : 1)
print(1 && 0 || 1)

print(0 ? fn: 1 ? "no" : "nop" : "yes")

!a ? a = 1 ? "yes" : "no" : "no!!"
print(a)

a = 1 && 1 ? a ? "hey" : "no" : 0

print(a)

ret

// -> "num: 0"
// -> "num: 1"
// -> "str: yes"
// -> "str: yes"
// -> "str: hey"
