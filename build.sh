#!/bin/sh

set -e

#export BOOST_INCLUDEDIR=$PWD/../boost
#export BOOST_LIBRARYDIR=$PWD/../boost/stage/lib

mkdir -p out
cd out
cmake ..
make -j8
