# ivm
ivm is a virtual machine built trying to improve the performance of a prototype-based language [ink](https://github.com/rod-lin/ink "ink")

## dependencies
    cmake >= 2.8
    gcc >= 4.8.4 or clang >= 3.4 (not really tested, but need support for c99 and labels-as-values extension)

## build
Build using cmake

    cmake ./
    
or

    cmake -DVERSION=release
    
for faster release version

Then use make to compile

    make

Exes and libs are generated in /build/bin and /build/lib dir in default

Tests(in /test dir) are available as well(written in ias, a visible version of bytecode in ivm)

    make test

## binary outputs

    build/bin/ias      parse and execute .ias file
    build/bin/ilang    parse, compile and execute an example language
    bulid/bin/testbed  general tests

<br><br>
ivm is still under development -- bugs everywhere.
