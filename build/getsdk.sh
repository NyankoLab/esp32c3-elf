#!/bin/bash

if [ ! -f "clang-esp-esp-19.1.2_20250312-aarch64-apple-darwin.tar.xz" ]; then 
  wget -N https://github.com/espressif/llvm-project/releases/download/esp-19.1.2_20250312/clang-esp-19.1.2_20250312-aarch64-apple-darwin.tar.xz
fi

if [ ! -d "../esp-clang" ]; then
  cd ..
  tar xvf build/clang-esp-19.1.2_20250312-aarch64-apple-darwin.tar.xz
  xattr -r -d com.apple.quarantine esp-clang
  mkdir esp-clang/include/c++
  ln -s /opt/homebrew/opt/llvm/include/c++/v1 esp-clang/include/c++/v1
  cd build
fi
