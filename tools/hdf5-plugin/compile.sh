#!/bin/bash
rm build/ -rf

PREFIX=$PWD/../../install/


./configure --prefix=$PREFIX --with-scil=$PREFIX --with-hdf5=$PREFIX

cd build/
make -j
make install

export HDF5_PLUGIN_PATH=$PREFIX/lib
