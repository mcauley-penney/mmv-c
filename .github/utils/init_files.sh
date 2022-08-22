#!/bin/sh
# cheatsheet: https://devhints.io/bash

main()
{
    dir="$1"
    num_files=$(test -n "$2" && echo "$2" || echo "10")

    mkdir -p "$dir"

    i=1
    while [ "$i" -le "$num_files" ]; do
        fname="$dir/test$i.txt"
        touch "$fname"
        echo "$fname" > "$fname"

        i=$(( i + 1 ))
    done
}

main "$@"
