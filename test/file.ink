import io

fp = io.file("test/text1")
print(fp.read())

print(fp is io.file)

fp.close()

fp = io.file("test/text2")

try: print(fp.read(1000)) catch: print("failed to read")
print(fp.len())
print(fp.len() == fp.read().len())

fp.seek()

for loc str in fp.lines().slice(-2, -1):
	print(str)

fp.seek()

fp.read(10)
print(fp.cur())
fp.seek()
print(fp.cur())

fp.read(20, true)
print(fp.cur())
fp.seek(10)
print(fp.cur())

nfp = fp.clone()

// file cannot be cloned
try: nfp.seek(10)
catch: print("yes")

fp.close()

fp = io.file("test/text3", "wb+")

fp.write("hello, world")
fp.seek()
print(fp.read())

io.remove("test/text3")

// io.stdout.write("hey\n")
// io.stderr.write("no line buffer")
// io.stderr.write("!\n")

try: io.stdin.write("no")
catch: print("of course")

// a = io.stdin.read()
// print(a)

// a = io.stdin.read(5)
// print(a)

ret

// <- "123456789"

// -> "str: content!"
// -> "num: 1"

// -> "str: failed to read"
// -> "num: 619"
// -> "num: 1"
// -> "str: So long lives this and this gives life to thee\\."

// -> "num: 10"
// -> "num: 0"
// -> "num: 0"
// -> "num: 10"
// 
// -> "str: yes"
// -> "str: hello, world"

// nop -> "hey"
// nop -> "no line buffer!"
// -> "str: of course"
