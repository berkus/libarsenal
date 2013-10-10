#!/usr/bin/env sh
# Install missing or update outdated Ubuntu packages for Travis VM.
sudo add-apt-repository -y ppa:h-rayflood/llvm
sudo add-apt-repository -y ppa:28msec/utils
sudo apt-get update
sudo apt-get install -qq --fix-missing libpulse-dev clang-3.3 cmake libboost-dev
