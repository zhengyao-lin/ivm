
buf = buffer(8)
buf.init()

print(1.to_s())
print("hello".to_s())
print(buf.to_s())
print(1.type().to_s())

// -> "str: 1"
// -> "str: hello"
// -> "str: 0x0000000000000000"
// -> "str: numeric"
