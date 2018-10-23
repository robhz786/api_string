#!/bin/bash

if ! [ -x "$CXX" ]; then
    echo "must set CXX environment variable to a valid compiler"
    exit 1
fi

for f in $(ls *.cpp | sed 's/.cpp$//g')
do
    echo -n compiling $f.cpp:
    time $CXX $CXXFLAGS -std=c++17 -I../include -c $f.cpp -o $f.o
    echo
done
    
