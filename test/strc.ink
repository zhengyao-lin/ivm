import std
import ulist
loc.merge(import struct, true)

s = struct([ array(byte, 10) ])
buf = s.pack("hello".ords().fill(10))
print(buf.to_s())

s = struct([ int, double, float, long, byte ])

buf = s.pack(2147483648, 1.2, 2.0, 100000000000000, 256)
s.unpack(buf).print()

s = s.clone()

// for i in range(1000000): 1 // gc

buf = s.pack(2147483648, 1.2, 2.0, 100000000000000, 256)
s.unpack(buf).print()

buf = buffer(1024)

s.packto(buf, 2147483648, 1.2, 2.0, 100000000000000, 256)
s.unpack(buf).print()

obj = [ int, double ]

del obj[1]

s = struct(obj)

s.packto(buf, 2147483648)
s.unpack(buf).print()

s = struct([ { type: int, count: 2 }, array(byte, 2), double, array(double, 5) ])

buf = s.pack([ 2147483648, 2 ], [ 256, 128 ], 10.1, [ i for loc i in range(5) ].map(fn x: x / 2))

print(s.size())

s.unpack(buf).print()

ret

// -> "str: 0x68656c6c6f0000000000"
// -> "str: \\[ -2147483648, 1\\.2, 2, 1e\\+14, 0 \\]"
// -> "str: \\[ -2147483648, 1\\.2, 2, 1e\\+14, 0 \\]"
// -> "str: \\[ -2147483648, 1\\.2, 2, 1e\\+14, 0 \\]"
// -> "str: \\[ -2147483648 \\]"
// -> "num: 58"
// -> "str: \\[ \\[ -2147483648, 2 \\], \\[ 0, 128 \\], 10.1, \\[ 0, 0.5, 1, 1.5, 2 \\] \\]"
