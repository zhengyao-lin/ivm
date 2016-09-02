print(1 || 0)									// -> "num: 1"
print(0 || 2)									// -> "num: 2"
print(0 && 1)									// -> "num: 0"
print(1 && 0)									// -> "num: 0"
print(0 && 0)									// -> "num: 0"
print(2 && 1)									// -> "num: 1"
print(1 && 1 || 1 && 0)							// -> "num: 1"
print(1 && 1 && 1 && 0)							// -> "num: 0"
print(1 && (1 || 0) && 1)						// -> "num: 1"
print(1 && (1 && 0) && 1)						// -> "num: 0"
print((1 && 0 || 0) && 1 || 2)					// -> "num: 2"
print((1 && ((1 && 1) && 0 || 0)) && 1 && 1)	// -> "num: 0"

if 1 && 1: print("yes!")						// -> "str: yes!"
if 1 && 0: print("no!")
if 0 && 0: print("no!")
if 1 && 0 || (1 && (0 || 1)): print("yes!")		// -> "str: yes!"
if 1 && (0 || (1 && (0 || 1))): print("yes!")	// -> "str: yes!"
