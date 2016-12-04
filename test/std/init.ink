import io

top.print = fn obj: {
	if obj is string:
		io.stdout.write("str: " + obj + "\n")
	elif obj is numeric:
		io.stdout.write("num: " + obj + "\n")
	elif !(obj is none) && obj.to_s is function:
		io.stdout.write(typename(obj) + ": " + obj.to_s() + "\n")
	else:
		io.stdout.write("<" + typename(obj) + ">\n")

	none
}
