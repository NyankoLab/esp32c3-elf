#!/bin/bash

if [ ! -f "clang-esp-20.1.1_20250829-aarch64-apple-darwin.tar.xz" ]; then 
  wget -N https://github.com/espressif/llvm-project/releases/download/esp-20.1.1_20250829/clang-esp-20.1.1_20250829-aarch64-apple-darwin.tar.xz
fi

if [ ! -d "../esp-clang" ]; then
  cd ..
  tar xvf build/clang-esp-20.1.1_20250829-aarch64-apple-darwin.tar.xz
  xattr -r -d com.apple.quarantine esp-clang
  mkdir esp-clang/include/c++
  ln -s /opt/homebrew/opt/llvm/include/c++/v1 esp-clang/include/c++/v1
# GenerateStringLiteral
#-GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
#+GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::None);
# printf '\x1f\x20\x03\xd5' | dd of=./clang-20 bs=1 seek=0x1A26304 conv=notrunc
# codesign -f -s - ./clang-20
  cd build
fi
