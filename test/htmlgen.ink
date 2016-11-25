
loc res = "<!DOCTYPE html>"
loc depth = 0
loc tab = "   "

loc def_tag = fn *tags: {
	for tag in tags:
		top[tag] = (fn tag: fn ct: {
			loc pref = tab * depth
			
			res += "\n"
			res += pref + "<" + tag + ">"

			if ct is function: {
				depth += 1

				if (loc tmp = ct()) is string:
					res += "\n" + pref + tab + tmp
				
				depth -= 1
			}

			res += "\n"
			res += pref + "</" + tag + ">"
			
			none
		})(tag)
}

def_tag("html", "head", "body", "h1", "script")

html {
	head {
		script {
			"console.log(a)"
		}
	}

	body {
		h1 {
			"title"			
		}
	}
}

print(res)

ret

// -> "str: <!DOCTYPE html>"
// -> "<html>"
// -> "   <head>"
// -> "      <script>"
// -> "         console.log\\(a\\)"
// -> "      </script>"
// -> "   </head>"
// -> "   <body>"
// -> "      <h1>"
// -> "         title"
// -> "      </h1>"
// -> "   </body>"
// -> "</html>"
