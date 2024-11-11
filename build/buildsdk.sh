#!/bin/bash

export BUILD_PATH=$PWD
export IDF_PATH=$PWD/../esp-idf
export PATH=$PWD:$PWD/../esp-clang/bin:$PWD/../riscv32-esp-elf/bin:$PATH

if [ ! -f "hello_world/README.md" ]; then
  cp -r $IDF_PATH/examples/get-started/hello_world .
fi

export IDF_TARGET=esp32c3
export IDF_MAINTAINER=1

cd hello_world
idf.py -D IDF_TOOLCHAIN=clang menuconfig
idf.py -D IDF_TOOLCHAIN=clang build
cd ..
