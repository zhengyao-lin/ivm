import io

fp = io.file("test/text1")
print(fp.read())

print(fp is io.file)

fp.close()

fp = io.file("test/text2")

try: print(fp.read(1000)) catch: print("failed to read")
print(fp.read(-1).len())
print(fp.read(-2).len())
print(fp.read(-620).len())
print(fp.read().len())

for loc str in fp.lines().slice(-2, -1):
	print(str)

fp.read(10, 0)
print(fp.cur())
fp.seek()
print(fp.cur())

fp.read(20, 0)
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

ret

// -> "str: content!"
// -> "num: 1"

// -> "str: failed to read"
// -> "num: 619"
// -> "num: 618"
// -> "num: 619"
// -> "num: 619"
// -> "str: So long lives this and this gives life to thee\\."

// -> "num: 11"
// -> "num: 0"
// -> "num: 21"
// -> "num: 10"
// 
// -> "str: yes"
// -> "str: hello, world"
