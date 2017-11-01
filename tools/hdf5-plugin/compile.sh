#!/bin/bash
rm build/ -rf

PREFIX=$PWD/../../install/


./configure --prefix=$PREFIX --with-scil=$PREFIX --with-hdf5=$PREFIX --with-cc=$(which gcc)

cd build/
make -j 4
make -j 4 install

export HDF5_PLUGIN_PATH=$PREFIX/lib
