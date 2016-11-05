import ulist

loc arrange = fn lst, n: {
	loc r = []
	loc dfs = fn comb, cur: {
		if cur == 0: {
			r.push(comb.clone())
			ret
		}

		cur = cur - 1
		for loc e in lst:
			if !comb.has(e): {
				comb.push(e)
				dfs(comb, cur)
				comb.pop()
			}
	}

	dfs([], n)
	ret r
}

arrange([ 1, 2, 3 ], 2).print()
arrange([ 1, 2, 3 ], 3).print()

// -> "str: \\[ \\[ 1, 2 \\], \\[ 1, 3 \\], \\[ 2, 1 \\], \\[ 2, 3 \\], \\[ 3, 1 \\], \\[ 3, 2 \\] \\]"
// -> "str: \\[ \\[ 1, 2, 3 \\], \\[ 1, 3, 2 \\], \\[ 2, 1, 3 \\], \\[ 2, 3, 1 \\], \\[ 3, 1, 2 \\], \\[ 3, 2, 1 \\] \\]"
