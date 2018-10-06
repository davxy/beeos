#!/bin/sh

filename=$1

# Be sure we are using unix file format
dos2unix -q $filename

# Replace tabs with 4 spaces
sed -i "s/\t/    /g" $filename

# Remove blanks at end of lines
sed -i "s/[[:blank:]]*$//" $filename

# Remove multiple ;
sed -i "s/;*;/;/g" $filename

# Find how many empty lines are present at end of file

set -- `awk 'BEGIN { tot=0; nl=0; }
        { tot+=length($0)+1; if(NF==0) { nl++; } else { nl=0; } }
        END { printf("%d %d\n", tot, nl) }' $filename`

# Discover multibyte characters

filesiz=$(stat --printf="%s" $filename)
if [ $1 -ne $filesiz ]
then
    echo
    echo "Multibyte characters in " $filename
    grep --color='auto' -P -n "[^\x00-\x7F]" $filename
fi

# Remove redundant empty lines at the end of file
truncate -s -$2 $filename

