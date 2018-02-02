#!/bin/bash

cwd=`pwd`
ef_cpp='../eyefinder_cpp'
cpp2py='../cpp2py'

echo -n "Remember: run ./run_eyefinder.sh before this ALWAYS!"
cd $cpp2py && python3 user.py
