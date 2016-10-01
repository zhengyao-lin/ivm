
print("testmod started")

// mod search order:
// 1. "<mod_name>.suf" in the current dir
// 2. "<mod_name>/init.suf"
// 
// suffix search order:
// 1. custom suffix defined by front-end(optional, e.g. .ink, .ias, .iobj)
// 2. dll/so

import search

// "import a.b.c.d" will create three objects a, b, c and set the final mod local object to the 'd' slot of a.b.c
import path1.path2.path3.path
print(path1.path2.path3.path.val)

// circular import

import circular
