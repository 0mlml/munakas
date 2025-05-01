#!/bin/bash

if ! command -v make &> /dev/null
then
    echo "make could not be found, please install it first."
    exit
fi

# build reader
cd reader
make

if [ $? -ne 0 ] || [ ! -f reader.out ]; then
    echo "Failed to build reader"
    exit 1
fi

cd ..

if ! command -v go &> /dev/null
then
    echo "go could not be found, please install it first."
    exit
fi

# build ui
cd ui
go build -o ui.out .

if [ $? -ne 0 ] || [ ! -f ui.out ]; then
    echo "Failed to build ui"
    exit 1
fi

# run ui passing in location of reader
./ui.out -reader ../reader/cs2radar.out