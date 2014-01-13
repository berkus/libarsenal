#!/usr/bin/env sh
# Install missing or update outdated Ubuntu packages for Travis VM.
dpkg --get-selections | grep hold

wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -
sudo add-apt-repository -y 'deb http://llvm.org/apt/precise/ llvm-toolchain-precise main'
#sudo add-apt-repository -y ppa:28msec/utils # Recent cmake
sudo apt-get update
sudo apt-get install -qq --fix-missing libpulse-dev clang-3.5 cmake libssl-dev
