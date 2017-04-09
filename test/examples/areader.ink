import std
import time
import io

println = print
print = fn str: (io.stdout.write(str), io.stdout.flush())

print("hello")

time.msleep(1000)

print("\rit's me.")

time.msleep(1000)

print("\n")

loc Passage = fn raw: {
	
}
