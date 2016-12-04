
import std
import ulist

print(list([ 1, 2, 3 ]))
print(list(range(1, 10)))
print(list())

try: print(list(1))
catch: print("failed")

ret

// -> "list: \\[ 1, 2, 3 \\]"
// -> "list: \\[ 1, 2, 3, 4, 5, 6, 7, 8, 9 \\]"
// -> "list: \\[\\]"
// -> "str: failed"
