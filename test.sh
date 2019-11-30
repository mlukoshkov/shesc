#!/bin/sh

cmdfile=test.cmds
npass=0
nfail=0

interpret()
{
	for arg; do
		printf "[%s]" "$arg"
	done
}

while read -r line; do
	printf "orig: [%s]\n" "$line"
	safeline=$(printf "%s" "$line" | ./shesc)
	printf "safe: [%s]\n" "$safeline"

	read -r expected
	actual=$(eval interpret "$safeline")

	if [ "$expected" != "$actual" ]; then
		printf "act : %s\n" "$actual"
		printf "exp : %s\n" "$expected"
		echo FAIL
		nfail=$((nfail+1))
	else
		echo PASS
		npass=$((npass+1))
	fi
	echo
done <$cmdfile

echo Passed: $npass
echo Failed: $nfail

test $nfail -eq 0
