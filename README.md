# ivm
ivm is a virtual machine built aiming to improve the performance of a prototype-based language [ink](https://github.com/rod-lin/ink "ink")

## dependencies
    cmake >= 2.8
    gcc >= 4.8.4 or clang >= 3.4

## build
Build using cmake

    cmake ./
    
or

    cmake -DVERSION=release
    
for faster releasing version

Then use make to compile

    make

Tests(in /test dir) are available as well

    make test

## executable files(in build/bin in default)

    build/bin/ias      parse and execute .ias file
    build/bin/ilang    parse, compile and execute an example language
    bulid/bin/testbed  general tests

<br><br>
ivm is still under development.
