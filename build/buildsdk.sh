#!/bin/bash

export ESP_MATTER_PATH=$PWD/../esp-matter
export HOMEKIT_PATH=$PWD/../esp-homekit-sdk
#export IDF_PATH=$PWD/../esp-idf
#export PATH=$PWD:$PWD/../esp-clang/bin:$PWD/../riscv32-esp-elf/bin:$PATH

if [ ! -f "patchsdk.ok" ]; then
  echo ok > patchsdk.ok
  cd ../esp-idf
  patch < ../build/patchsdk
  cd ../build
fi

if [ ! -f "patchhomekit.ok" ]; then
  echo ok > patchhomekit.ok
  cd ../esp-homekit-sdk
  patch < ../build/patchhomekit
  cd ../build
fi

if [ ! -f "patchmatter.ok" ]; then
  echo ok > patchmatter.ok
  cd ../esp-matter
  patch < ../build/patchmatter
  cd ../build
fi

#if [ ! -f "hello_world/README.md" ]; then
#  cp -r $IDF_PATH/examples/get-started/hello_world .
#fi

if [ ! -f "light/README.md" ]; then
  cp -r $ESP_MATTER_PATH/examples/light .
fi

#export IDF_TARGET=esp32c3
#export IDF_MAINTAINER=1

cd light
idf.py -DIDF_TOOLCHAIN=clang set-target esp32c3
cp ../sdkconfig.esp32c3 sdkconfig
idf.py -DIDF_TOOLCHAIN=clang menuconfig
idf.py -DIDF_TOOLCHAIN=clang build
cd ..
