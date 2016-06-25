#! /bin/bash

function clean()
{
	if [ -d CMakeFiles ]; then
		rm -r CMakeFiles
	fi

	if [ -d Testing ]; then
		rm -r Testing
	fi

	if [ -f CMakeCache.txt ]; then
		rm CMakeCache.txt
	fi

	if [ -f cmake_install.cmake ]; then
		rm cmake_install.cmake
	fi

	if [ -f CTestTestfile.cmake ]; then
		rm CTestTestfile.cmake
	fi

	if [ -n $1 ] && [ "$1" = "dist" ] && [ -f Makefile ]; then
		rm Makefile
	fi

	for file in `ls`
	do
		if [ -d "$file" ] && [ "$file" != ".git" ]; then
			cd $file
			clean $@
			cd ..
		fi
	done
}

if [ -d build ]; then
	rm -r build
fi

clean $1
