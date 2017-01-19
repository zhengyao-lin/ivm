// coroutine
loc c = fork fn [ (yield i * 2) for loc i in range(10) ]

while c.alive():
	print(resume c)
