// Christmas Bug !!!

loc fx = 0
loc key = 0

resume fork: {
	fx = fn: {
		loc a = key
	}
}

fx()

for loc j in range(1000000): none

fx() // the vm may stuck in here because of the circular reference of context(ctx->prev == ctx)

ret
