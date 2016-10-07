import ulist
loc.merge(import struct, true)

s = struct {
	v1: int,
	v2: double,
	v3: float,
	v4: long
}

buf = s.pack(2147483648, 1.2, 2.0, 100000000000000)
s.unpack(buf).print()

s = s.clone()

for i in range(1000000): 1 // gc

buf = s.pack(2147483648, 1.2, 2.0, 100000000000000)
s.unpack(buf).print()

buf = buffer(1024)

s.packto(buf, 2147483648, 1.2, 2.0, 100000000000000)
s.unpack(buf).print()

ret

// -> "str: \\[ -2147483648, 1\\.2, 2, 1e\\+14 \\]"
// -> "str: \\[ -2147483648, 1\\.2, 2, 1e\\+14 \\]"
// -> "str: \\[ -2147483648, 1\\.2, 2, 1e\\+14 \\]"
