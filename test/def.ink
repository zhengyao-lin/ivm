loc def = fn a = 10: {
	print(a)
}

def(20)
def(none)
def()

loc def2 = fn a = "hello", b = ", world", c = 33.char(): {
	print(a + b + c)
}

def2(none, ", yo")
def2()

import ulist

// default parameters are dynamically generated
loc def3 = fn vals = [ 1, 2, 3, 4 ]: {
	vals.print()
	vals[0] = "wrong!"
}

def3()
def3()

loc def4 = fn *vargs, a = "hey": {
	print(a)
	vargs.print()
}

def4()
def4("wow")
def4("here", "wow")

ret

// -> "num: 20"
// -> "num: 10"
// -> "num: 10"
// -> "str: hello, yo!"
// -> "str: hello, world!"
// -> "str: \\[ 1, 2, 3, 4 \\]"
// -> "str: \\[ 1, 2, 3, 4 \\]"
// -> "str: hey"
// -> "str: \\[\\]"
// -> "str: wow"
// -> "str: \\[\\]"
// -> "str: wow"
// -> "str: \\[ here \\]"
