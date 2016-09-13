import math
import ulist

i = 0
wrong = 0

loc RChecker = fn min, max, step: {
	step = step || 0.5
	loc a = (max - min) / step
	loc counts = [ 0 ] * a.ceil()
	loc errors = []
	loc sum = 0
	loc count = 0

	{
		acc: fn val: {
			loc bd = min
			loc i = 0
			loc set = 0

			sum = sum + val
			count = count + 1

			while i < a: {
				if val >= bd &&
				   val < bd + step: { // found range
				   	counts[i] = counts[i] + 1
				   	set = 1
					break
				}

				i = i + 1
				bd = bd + step
			}

			if !set: {
				if val < max && val >= min:
					counts[a - 1] = counts[a - 1] + 1
				else:
					errors.push(val)
			}
		},

		print: fn: {
			print("results(step " + step +  ", average: " + (sum / count) + "):")
			loc bd = min
			loc i = 0
			loc pref = "   from "

			while i < a: {
				if i != a - 1:
					print(pref + bd + " to " + (bd + step) + ": " + counts[i])
				else:
					print(pref + bd + " to " + max + ": " + counts[i])

				i = i + 1
				bd = bd + step
			}

			print(errors.size() + " errors")

			ret
		},

		average: fn: sum / count,
		has_error: fn: errors.size()
	}
}

loc min = -10
loc max = 15

chk = RChecker(min, max, 1)

i = 0

while i < 50000: {
	chk.acc(math.random(min, max))
	i = i + 1
}

// chk.print()

if !chk.has_error():
	print("no error")
else:
	print("error")


if math.abs(chk.average() - (max + min) / 2) < 0.1:
	print("correct average")
else:
	print("average error")

ret

// -> "str: no error"
// -> "str: correct average"
