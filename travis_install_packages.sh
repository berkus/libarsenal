#!/usr/bin/env sh
# Install missing or update outdated Ubuntu packages for Travis VM.
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -
sudo add-apt-repository -y 'deb http://llvm.org/apt/precise/ llvm-toolchain-precise main'
sudo add-apt-repository -y ppa:28msec/utils
sudo apt-get update
sudo apt-get install -qq --fix-missing libpulse-dev clang-3.4 cmake libboost-all-dev

apt-file list clang-3.4

clang --version
clang-3.4 --version
