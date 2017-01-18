import ulist
import math
import sys

loc bmap = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
loc brmap = bmap.chars().reduce(fn t, k: (t[k] = t.index += 1, t), { index: -1 })

loc base64_encode = fn str: {
	loc l = str.len()
	loc fin = ""

	for loc i in range(0, l, 3): {
		loc c1 = (str[i].ord()) & 0xff
		loc c2 = i + 1 < l ? str[i + 1].ord() & 0xff : none
		loc c3 = i + 2 < l ? str[i + 2].ord() & 0xff : none
		
		fin +=
			bmap[c1 >>> 2] +
			bmap[(c2 == none ? 0 : (c2 >>> 4)) | ((c1 & 0b11) << 4)] +
			(c2 == none ? "=" : bmap[(c3 == none ? 0 : (c3 >>> 6)) | ((c2 & 0b1111) << 2)]) +
			(c3 == none ? "=" : bmap[c3 & 0b111111])
	}

	fin
}

loc base64_decode = fn str: {
	loc l = str.len()
	loc fin = ""

	assert (l & 0b11) == 0 // l % 4 == 0

	for loc i in range(0, l, 4): {
		loc c1 = brmap[str[i]]
		loc c2 = brmap[str[i + 1]]
		loc c3 = brmap[str[i + 2]]
		loc c4 = brmap[str[i + 3]]

		assert
			c1 != none && c2 != none &&
			(c3 != none || str[i + 2] == "=") &&
			(c4 != none || str[i + 3] == "=")

		fin += (c1 << 2 | ((c2 & 0b00110000) >>> 4)).char()
		fin += (c2 << 4 | (c3 == none ? break : ((c3 & 0b00111100) >>> 2))).char()
		fin += (c3 << 6 | (c4 == none ? break : c4 & 0b00111111)).char()
	}

	fin
}

if sys.argv.size() > 1: {
	loc arg = sys.argv.slice(1)
	loc f = arg.has("-d") ? base64_decode : base64_encode

	for loc s in arg: {
		if s != "-d":
			print(s + " -> " + f(s))
	}
} else {
	loc rstr = [ [ math.random(32, 126).char() for loc i in range(0, math.random(16, 512)) ].sum() for loc c in range(500) ]

	for loc s in rstr: {
		loc es = base64_encode(s)
		loc ds = base64_decode(es)

		if ds != s: {
			print("wrong: " + s)
		}
	}
}

// print(base64_encode("0d0e58888dsajklfds"))
// print(base64_decode(base64_encode("0d0e58888dsajklfds")))

ret
