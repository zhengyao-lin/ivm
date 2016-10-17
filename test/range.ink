for loc i in range(1, 3):
	print(i)

print("#####")

for loc i in range(2, 1):
	print(i)

print("#####")

for loc i in range(-2, 1):
	print(i)

print("#####")

for loc i in range(0, 9, 2):
	print(i)

print("#####")

for loc i in range(0, 11, 2):
	print(i)

print("#####")

for loc i in range(0, 15, 3):
	print(i)

print("#####")

for loc i in range(0, -15, -3):
	print(i)

print("#####")

for loc i in range(0, 0):
	print(i)

print("#####")

for loc i in range(5):
	print(i)

print("#####")

for loc i in range(-10):
	print(i)

print("#####")

for loc i in range(, 5,):
	print(i)

print("#####")

for loc i in range(1, 5,):
	print(i)

print("#####")

for loc i in range(, 5):
	print(i)


range_iter.proto.next = fn: {
	print("here we are")
	raise 0
}

list_iter.proto.next = fn: {
	print("good")
	raise 0
}

for loc i in range(100):
	print("no!")

for loc i in [ 0, 1 ]:
	print("nop!!")

// -> "num: 1"
// -> "num: 2"
// -> "str: #####"
// -> "str: #####"
// -> "num: -2"
// -> "num: -1"
// -> "num: 0"
// -> "str: #####"
// -> "num: 0"
// -> "num: 2"
// -> "num: 4"
// -> "num: 6"
// -> "num: 8"
// -> "str: #####"
// -> "num: 0"
// -> "num: 2"
// -> "num: 4"
// -> "num: 6"
// -> "num: 8"
// -> "num: 10"
// -> "str: #####"
// -> "num: 0"
// -> "num: 3"
// -> "num: 6"
// -> "num: 9"
// -> "num: 12"
// -> "str: #####"
// -> "num: 0"
// -> "num: -3"
// -> "num: -6"
// -> "num: -9"
// -> "num: -12"
// -> "str: #####"
// -> "str: #####"
// -> "num: 0"
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "str: #####"
// -> "str: #####"
// -> "num: 0"
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "str: #####"
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "str: #####"
// -> "num: 0"
// -> "num: 1"
// -> "num: 2"
// -> "num: 3"
// -> "num: 4"
// -> "str: here we are"
// -> "str: good"
