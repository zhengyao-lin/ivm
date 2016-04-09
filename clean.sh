#! /bin/bash

function clean()
{
	if [ -d CMakeFiles ]; then
		rm -r CMakeFiles
	fi

	if [ -f CMakeCache.txt ]; then
		rm CMakeCache.txt
	fi

	if [ -f cmake_install.cmake ]; then
		rm cmake_install.cmake
	fi

	if [ -n $1 ] && [ "$1" = "dist" ] && [ -f Makefile ]; then
		rm Makefile
	fi

	for file in `ls`
	do
		if [ -d "$file" ]; then
			cd $file
			clean
			cd ..
		fi
	done
}

if [ -d build ]; then
	rm -r build
fi

clean $1
