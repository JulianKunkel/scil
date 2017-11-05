#!/bin/bash -e

PREFIX=$PWD/../../../install/

gfortran -c scil-mod.f90 -g3  -I$PREFIX/include

mpicc -c scil-fortran-interface.c -g3 -I$PREFIX/include

$PREFIX/bin/h5pfc -c test.f90 -g3 -I$PREFIX/include

gfortran -g3 test.o scil-fortran-interface.o scil-mod.o -o test.exe -L$PREFIX/lib -lhdf5_fortran -lhdf5 -lscil -lhdf5-filter-scil -Wl,--rpath=$PREFIX/lib

export HDF5_PLUGIN_PATH=$PREFIX/lib
./test.exe

# src/H5pubconf.h:#define H5_DEFAULT_PLUGINDIR "/usr/local/hdf5/lib/plugin"
