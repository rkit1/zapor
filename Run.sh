#!/bin/bash

(
flock -n 200 || exit 1;
cd "`dirname \"$0\"`";
./bin/zapor_clicker;
) 200>/var/lock/zapor;