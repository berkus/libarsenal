#!/usr/bin/env sh
# Install missing or update outdated Ubuntu packages for Travis VM.
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -
sudo add-apt-repository -y 'deb http://llvm.org/apt/precise/ llvm-toolchain-precise main'
sudo add-apt-repository -y ppa:28msec/utils
sudo apt-get update
sudo apt-get install -qq --fix-missing libpulse-dev clang-3.4 cmake

# Don't waste time installing wrong boost...
# libboost-all-dev

# which clang
# /usr/bin/clang --version

# Default clang installation is in /usr/local/bin/clang
#
# clang-3.4 installs the following binaries:
# /usr/bin/clang
# /usr/bin/clang-check
# /usr/bin/clang++
# /usr/bin/pollycc
# /usr/bin/asan_symbolize
# /usr/bin/scan-build
