#!/bin/bash
rm build/ -rf

SCIL=/home/anastasiia/sw/install
HDF5=/home/anastasiia/git/scil/tools/hdf5-plugin/test/fortran/install


./configure --prefix=$SCIL --with-scil=$SCIL --with-hdf5=$HDF5

cd build/
make -j
make install

export HDF5_PLUGIN_PATH=$SCIL/lib
