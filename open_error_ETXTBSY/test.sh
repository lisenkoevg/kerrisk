#!/bin/bash

EXE=$1
expected1="ERROR [ETXTBSY Text file busy] opening file ${EXE}"
expected2="ERROR [EFAULT Device or resource busy] opening file open_error_ETXTBSY"
actual="$(./${EXE} ${EXE} 2>&1)"
echo expected1=$expected1
echo expected2=$expected2
echo actual = $actual
test "$expected1" = "$actual" -o "$expected2" = "$actual" && echo Test ok || echo Test failed
