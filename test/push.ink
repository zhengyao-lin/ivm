list = [1, 2, 3, 4]
size = list.size()
i = 0

while i < 1000000: {
	list.push(i)
	i = i + 1
}

if list.size() - size == i:
	print("yes") // -> "str: yes"

try: list.push()
catch: print("right") // -> "str: right"
