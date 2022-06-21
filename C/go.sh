#!/bin/bash
make clean
make CC=gcc ARCH=x64
./kex_test
#ddd kex_test
