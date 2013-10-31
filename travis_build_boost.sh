#!/usr/bin/env sh
# Build a 64-bit c++11 libc++ based boost libraries we need for linux.
# Based on brew recipe.

set -x

wget -O boost_1_54_0.tar.bz2 http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.bz2/download
tar xjf boost_1_54_0.tar.bz2
cd boost_1_54_0
cat <<EOF > user-config.jam
using clang-linux : : /usr/bin/clang++ ;
EOF
export BOOST_DIR=/usr/local/opt/boost
./bootstrap.sh --prefix=$BOOST_DIR --libdir=$BOOST_DIR/lib64 \
	--with-toolset=clang \
	--with-libraries=system,date_time,program_options,test \
	--without-icu
sudo ./b2 --prefix=$BOOST_DIR --libdir=$BOOST_DIR/lib64 -d0 -j6 --layout=system \
	--user-config=user-config.jam threading=multi install toolset=clang \
	cxxflags=-std=c++11 cxxflags=-stdlib=libc++ cxxflags=-fPIC cxxflags=-m64 \
	linkflags=-stdlib=libc++ linkflags=-m64
