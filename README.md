# ivm
ivm is a simple vm built for a prototype-based language [ink](https://github.com/rod-lin/ink "ink")

## dependencies
    cmake >= 2.8
	gcc >= 4.8.4 or clang >= 3.4

## build
Build using cmake

	cmake -DVERSION=release
	make
	make test

Further build options can be found in CMakefile.txt(e.g. -DPLATFORM)

## executable files(in build/bin in default)

    build/bin/ias      execute .ias file
    build/bin/ilang    execute .ink file
    bulid/bin/testbed  general tests
