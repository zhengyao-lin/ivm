
buf = buffer(100000)

print(buf.size())

try: buf = buffer(10000000000000)
catch: print("failed to alloc buffer")

try: buf = buffer(-1)
catch: print("failed to alloc buffer")

ret

// -> "num: 100000"
// -> "str: failed to alloc buffer"
// -> "str: failed to alloc buffer"
