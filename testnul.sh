#!/bin/sh

cmdfile=testnul.cmds
npass=0
nfail=0

getline()
{
	sed -n "$1{p;q;}" $cmdfile | tr -d '\n'
}

hex()
{
	od -An -tx1 -w32 | sed s/^.//
}

# read cannot read nul-bytes ever using -r, so we have to use sed.
nlines=$(wc -l <$cmdfile)
i=1
while [ $i -le $nlines ]; do
	printf "orig: ["
	getline $i
	echo "]"
	printf "ohex: "
	getline $i | hex
	printf "safe: ["
	getline $i | ./shesc
	echo "]"
	actual=$(getline $i | ./shesc | hex)
	echo "shex: $actual"
	expected=$(getline $((i+1)))
	if [ "$expected" != "$actual" ]; then
		echo "exp : $expected"
		echo FAIL
		nfail=$((nfail+1))
	else
		echo PASS
		npass=$((npass+1))
	fi
	echo
	i=$((i+2))
done

echo Passed: $npass
echo Failed: $nfail

test $nfail -eq 0
