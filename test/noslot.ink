import std

loc echo = {
	noslot: fn name: {
		print(name)
		base
	}
}

echo hi wow xixixi balabala
echo how are you

try: { noslot: 1 }.hey
catch: print("ha")

// -> "str: hi"
// -> "str: wow"
// -> "str: xixixi"
// -> "str: balabala"
// -> "str: how"
// -> "str: are"
// -> "str: you"
// -> "str: ha"
