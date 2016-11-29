import io
import math

loc ans = math.random(0, 101).trunc()

print = fn msg: io.stdout.write(msg + "\n")

while 1: {
	try: {
		loc guess = numeric(input("guess a number: "))
		if guess != guess.trunc() ||
		   guess < 0 || guess > 100:
		   raise "illegal guess"
	} catch: {
		print("illegal number, try again")
		cont
	}

	if guess > ans:
		print("too big")
	elif guess < ans:
		print("too small")
	else: {
		print("right!")
		break
	}
}
