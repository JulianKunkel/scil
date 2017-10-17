#!/bin/bash

echo "We assume you have installed HDF5 already with the SCIL plugin"

wget ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4.4.1.1.tar.gz
tar -xf netcdf*

INSTALL=$PWD/../../install
export CFLAGS="-I$INSTALL/include"
export CPPFLAGS=$CFLAGS
export LDFLAGS="-L$INSTALL/lib -lhdf5-filter-scil  -lscil"

cd netcdf*
patch -p1 < ../*patch
CC=mpicc ./configure --prefix=$INSTALL
