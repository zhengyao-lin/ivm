import ulist
import struct

s = struct.struct({
	v1: struct.int,
	v2: struct.double,
	v3: struct.float,
	v4: struct.long
})

buf = s.pack(2147483648, 1.2, 2.0, 100000000000000)
s.unpack(buf).print()

// -> "str: \\[ -2147483648, 1\\.2, 2, 1e\\+14 \\]"

ret
