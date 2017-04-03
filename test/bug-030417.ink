import std

print(0x80000001 | 0)
print(-1 >> 0)
print(-1 >>> 0)

ret

// -> "num: -2147483647"
// -> "num: -1"
// -> "num: 4294967295"
