import time
import mthread

loc i = 0

mthread.spawn(fork: {
	print("######################### bp")
	import test.bp
})

mthread.spawn(fork: {
	print("######################### test")
	import test
})

mthread.spawn(fork: {
	print("######################### sort")
	import test.sort
})

mthread.spawn(fork: {
	print("######################### gc")
	import test.gc
})

mthread.spawn(fork: {
	print("######################### testm")
	import test.testm
})

mthread.spawn(fork: {
	print("######################### ga")
	import test.ga
})

mthread.spawn(fork: {
	print("######################### fib")
	import test.fib
})

mthread.spawn(fork: {
	print("######################### dfa")
	import test.dfa
})

mthread.spawn(fork: {
	print("######################### huge")
	import test.huge
})

mthread.spawn(fork: {
	print("######################### lcomp")
	import test.lcomp
})

mthread.spawn(fork: {
	print("######################### list")
	import test.list
})

mthread.spawn(fork: {
	print("######################### intr")
	import test.intr
})

mthread.spawn(fork: {
	print("######################### mem")
	import test.mem
})

mthread.spawn(fork: {
	print("######################### varg")
	import test.varg
})

mthread.spawn(fork: {
	print("######################### unpack")
	import test.unpack
})

mthread.spawn(fork: {
	fib = fn n {
		if n < 2: ret 1
		ret fib(n - 1) + fib(n - 2)
	}

	print(fib(30))
})

// while end != 2: none

print(i)

mthread.join()

print("all end")

mthread.spawn(fork fn arg: {
	print(arg)
}, "hello")

ret