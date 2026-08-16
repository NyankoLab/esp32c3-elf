#!/bin/bash

if [ ! -f "clang-esp-21.1.3_20260408-aarch64-apple-darwin.tar.xz" ]; then 
  wget -N https://github.com/espressif/llvm-project/releases/download/esp-21.1.3_20260408/clang-esp-21.1.3_20260408-aarch64-apple-darwin.tar.xz
fi

if [ ! -d "../esp-clang" ]; then
  cd ..
  tar xvf build/clang-esp-21.1.3_20260408-aarch64-apple-darwin.tar.xz
  xattr -r -d com.apple.quarantine esp-clang
# mkdir esp-clang/include/c++
# ln -s /opt/homebrew/opt/llvm/include/c++/v1 esp-clang/include/c++/v1
# llvm-project/clang/lib/CodeGen/CodeGenModule.cpp
# GenerateStringLiteral
#-GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
#+GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::None);
# 09751812 AND             W9, W8, #0xFFFFFF3F
#-29011932 ORR             W9, W9, #0x80
#+1F2003D6 NOP
# printf '\x1f\x20\x03\xd5' | dd of=./clang-20 bs=1 seek=0x1A26304 conv=notrunc
# codesign -f -s - ./clang-20
# printf '\x1f\x20\x03\xd5' | dd of=./clang-21 bs=1 seek=0x19FD4A4 conv=notrunc
# codesign -f -s - ./clang-21
  cd build
fi
