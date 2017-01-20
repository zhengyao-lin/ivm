import sys

numeric.proto.~> = fn lst: base >= lst[0] && base < lst[1]

loc subt = {
	math: fn wrong: {
		loc raw = (50 - 1.25 * wrong).round()

		if raw < 0: ret "seriously?"

		loc fin = 800 - 10 * (44 - raw)
		fin >= 800 ? 800 : (fin < 0 ? 0 : fin)
	},

	physics: fn wrong: {
		loc raw = 75 - 1.25 * wrong

		if wrong == 0: "absolutely 800"
		elif raw ~> [60, 75]: "approx. 800"
		elif raw ~> [55, 60]: "770 ~ 790"
		elif raw ~> [50, 55]: "730 ~ 760"
		elif raw ~> [45, 50]: "700 ~ 720"
		elif raw ~> [40, 45]: "660 ~ 690"
		else "eh..." 
	}
}

loc help = fn msg = none: {
	if msg: print(msg)
	print("usage:\n   " + sys.argv[0] + " <subject> <how many wrong>")
	sys.exit(1)
}

if sys.argv.size() < 3:
	help()

loc subj = sys.argv[1]
loc wrong = numeric(sys.argv[2])

loc calc = subt[subj]

if !(calc is function):
	help("no such subject `" + subj + "`")

print("predict score: " + calc(wrong))

ret
