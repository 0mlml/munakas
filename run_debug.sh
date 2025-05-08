#!/bin/bash

CURRENT_DIR=$(pwd)

rm -f libmunakas.so munakas

echo "Compiling C code with debug symbols..."
gcc -g -O0 -fPIC -shared -o libmunakas.so c/cs.c c/process.c c/bridge.c -I. -lm -Wall

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CURRENT_DIR

echo "Compiling Go code with debug symbols..."
go build -gcflags="all=-N -l" -o munakas main.go reader.go parser.go

echo "Starting GDB..."
gdb --args ./munakas