#!/bin/sh

dump_args()
{
	i=0
	for arg; do
		echo "$i: $arg"
		i=$((i+1))
	done
}

escape() { printf "%s" "$1" | ./shesc; }

opts='--file "/path/Damn you, spaces!/"'

echo Command:
echo dump_args $opts

echo
echo Problem:
dump_args $opts

echo
echo Solution:
eval dump_args $(escape "$opts")
