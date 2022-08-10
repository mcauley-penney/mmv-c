#!/bin/bash
# cheatsheet: https://devhints.io/bash


main()
{
    local bin_name
	bin_name="mmv_test"

	# pass name of executable to cmake
	cmake -DPROJECT_NAME="${bin_name}" -S . build/
	cd build || exit
	make
	printf "\n\n\n"
	./"${bin_name}"
}


main "$@"
