#!/bin/bash

echo "We assume you have installed HDF5 already with the SCIL plugin"

if [[ ! -e  netcdf-4.4.1.1.tar.gz ]] ; then
  wget ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4.4.1.1.tar.gz
  tar -xf netcdf*
fi

INSTALL=$PWD/../../install
export CFLAGS="-I$INSTALL/include"
export CPPFLAGS=$CFLAGS
export LDFLAGS="-L$INSTALL/lib -lhdf5-filter-scil  -lscil -lscil-util"

if  [[ ! -e "$INSTALL/lib/libnetcdf.so" ]] ; then
  pushd netcdf*
  patch -p1 < ../*patch
  CC=mpicc ./configure --prefix=$INSTALL
  popd
fi

gcc  test-netcdf4.c  $CFLAGS -lnetcdf $LDFLAGS   -o test-netcdf4 -Wl,--rpath=$INSTALL/lib
rm *.nc
./test-netcdf4
h5dump -H -p tst_chunks3.nc
