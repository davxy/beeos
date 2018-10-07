#!/bin/bash

dirs=$1

if [[ $dirs == "" ]]
then
    dirs="../kernel/src ../libc/src ../libc/include ../libu/src ../libu/include ../user/src"
fi
echo "Checking $dirs"

# Replace tabs with 4 spaces
find $dirs -not -path '*/\.*' -type f | grep -E ".(h|c)$" | xargs -L1 ./stylecheck.sh
