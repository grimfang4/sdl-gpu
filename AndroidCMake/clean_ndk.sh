#!/bin/sh

(cd cmake_ndk_build
	for d in */ ; do
		(cd "$d"
			make clean
		)
	done
)

