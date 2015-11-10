#!/bin/sh
set -e
if [ ! -d "$HOME/libsodium/lib" ]; then
    wget https://github.com/jedisct1/libsodium/releases/download/1.0.6/libsodium-1.0.6.tar.gz
    tar xzf libsodium-1.0.6.tar.gz
    cd libsodium-1.0.6 && ./configure --prefix=$HOME/libsodium && make && make check && make install
else
    echo 'Using cached libsodium directory.'
fi
