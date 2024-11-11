#!/bin/bash

if [ ! -f "clang-esp-18.1.2_20240912-aarch64-apple-darwin.tar.xz" ]; then 
  wget -N https://github.com/espressif/llvm-project/releases/download/esp-18.1.2_20240912/clang-esp-18.1.2_20240912-aarch64-apple-darwin.tar.xz
fi

if [ ! -d "../esp-clang" ]; then
  cd ..
  tar xvf build/clang-esp-18.1.2_20240912-aarch64-apple-darwin.tar.xz
  xattr -r -d com.apple.quarantine esp-clang
  mkdir esp-clang/include/c++
  ln -s /opt/homebrew/opt/llvm/include/c++/v1 esp-clang/include/c++/v1
  cd build
fi

if [ ! -f "patchsdk.ok" ]; then
  echo ok > patchsdk.ok
  cd ../esp-idf
  patch < ../build/patchsdk
fi

if [ ! -d "../esp-idf/components/esp_hap_apple_profiles" ]; then ln -s ../../esp-homekit-sdk/components/homekit/esp_hap_apple_profiles ../esp-idf/components/esp_hap_apple_profiles; fi
if [ ! -d "../esp-idf/components/esp_hap_core" ]; then ln -s ../../esp-homekit-sdk/components/homekit/esp_hap_core ../esp-idf/components/esp_hap_core; fi
if [ ! -d "../esp-idf/components/esp_hap_extras" ]; then ln -s ../../esp-homekit-sdk/components/homekit/esp_hap_extras ../esp-idf/components/esp_hap_extras; fi
if [ ! -d "../esp-idf/components/esp_hap_platform" ]; then ln -s ../../esp-homekit-sdk/components/homekit/esp_hap_platform ../esp-idf/components/esp_hap_platform; fi
if [ ! -d "../esp-idf/components/hkdf-sha" ]; then ln -s ../../esp-homekit-sdk/components/homekit/hkdf-sha ../esp-idf/components/hkdf-sha; fi
if [ ! -d "../esp-idf/components/json_generator" ]; then ln -s ../../esp-homekit-sdk/components/homekit/json_generator ../esp-idf/components/json_generator; fi
if [ ! -d "../esp-idf/components/json_parser" ]; then ln -s ../../esp-homekit-sdk/components/homekit/json_parser ../esp-idf/components/json_parser; fi
if [ ! -d "../esp-idf/components/mu_srp" ]; then ln -s ../../esp-homekit-sdk/components/homekit/mu_srp ../esp-idf/components/mu_srp; fi
