#!/bin/bash
find *.png | while read in;
 do name=$(echo $in | cut -d "." -f 1); 
name1=$(echo $name |tail -c 2)
name2=$(echo $name | cut -c1-1)


./colorbalance -c green -a 50 "$0" "./converted/$1"
echo $name;
done;

