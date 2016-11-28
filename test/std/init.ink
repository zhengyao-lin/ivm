import io

top.print = fn obj: {
	if obj is string:
		io.stdout.write("str: " + obj + "\n")
	elif obj is numeric:
		io.stdout.write("num: " + obj + "\n")
	else:
		io.stdout.write("<" + typename(obj) + ">\n")

	none
}
