#!/bin/bash

. ~/lib_color.sh

EXE=$1
expected="____aaa 1 thread: \\d+$"
actual="$(./${EXE} ${EXE} 2>&1)"
echo expected="'"$expected"'"
echo actual = "'"$actual"'"
echo "$actual" | grep -qP "$expected" && echo_green "Test ok" || echo_red "Test failed"
