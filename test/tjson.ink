import std
import ulist
import json

print(json.encode({ hello: "hey", num: 1, list: [ 1, 2, 3, 4 ], none: none }))

// print(json.parse("\"  \nhe\\nll\\to\"  "))
// print(json.parse("\"\""))

// print(json.parse(" \t\n\r \r"))
// print(json.parse(" \t\n .0"))

print(json.encode(json.decode("[ 1, 2, \"hello\", [], [ null, 1 ], [ 1, 2, 3e10, [[1e-1, { \"s\": 1, \"key\":{} }]] ] ]   \n")))
// print(json.decode("\"\\uD800\\u\""))

ret

// -> "str: \\{"hello":"hey","num":1,"list":\\[1,2,3,4\\],"none":null\\}"
// -> "str: \\[1,2,"hello",\\[\\],\\[null,1\\],\\[1,2,3e\\+10,\\[\\[0.1,\\{"s":1,"key":\\{\\}\\}\\]\\]\\]\\]"
