#!/bin/bash

(
flock -n 200 || exit 1;
cd "`dirname \"$0\"`";
./dist/Debug/GNU-Linux-x86/cppapplication_1;
) 200>/var/lock/zapor;