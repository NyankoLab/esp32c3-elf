#!/bin/bash

export ESP_MATTER_PATH=$PWD/../esp-matter
#export IDF_PATH=$PWD/../esp-idf
#export PATH=$PWD:$PWD/../esp-clang/bin:$PWD/../riscv32-esp-elf/bin:$PATH
export PATH=$PWD:$PWD/../esp-clang/bin:$PATH

if [ ! -f "patch/esp-idf.ok" ]; then
  echo ok > patch/esp-idf.ok
  cd ../esp-idf
  patch < ../build/patch/esp-idf.patch
  cd ../build
fi

if [ ! -f "patch/esp-matter.ok" ]; then
  echo ok > patch/esp-matter.ok
  cd ../esp-matter
  patch < ../build/patch/esp-matter.patch
  cd ../build
fi

if [ ! -f "patch/connectedhomeip.ok" ]; then
  echo ok > patch/connectedhomeip.ok
  cd ../esp-matter/connectedhomeip/connectedhomeip
  patch < ../../../build/patch/connectedhomeip.patch
  cd ../../../build
fi

#if [ ! -f "hello_world/README.md" ]; then
#  cp -r $IDF_PATH/examples/get-started/hello_world .
#fi

if [ ! -f "light/README.md" ]; then
  cp -r $ESP_MATTER_PATH/examples/light .
  cd light
  idf.py add-dependency "espressif/cjson"
  cd ..
fi

export IDF_TARGET=esp32c3
export IDF_MAINTAINER=1

cd light
idf.py -DIDF_TOOLCHAIN=clang update-dependencies
idf.py -DIDF_TOOLCHAIN=clang set-target esp32c3
cp ../sdkconfig.esp32c3 sdkconfig
idf.py -DIDF_TOOLCHAIN=clang menuconfig
idf.py -DIDF_TOOLCHAIN=clang build
cd ..
