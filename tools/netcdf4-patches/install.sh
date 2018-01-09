#!/bin/bash

echo "We assume you have installed HDF5 already with the SCIL plugin"

if [[ ! -e  netcdf-4.4.1.1.tar.gz ]] ; then
  wget ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4.4.1.1.tar.gz
  tar -xf netcdf*
fi

PREFIX=$PWD/../../install/
export CFLAGS="-I$PREFIX/include"
export CPPFLAGS=$CFLAGS
export LDFLAGS="-L$PREFIX/lib -lhdf5 -lhdf5_hl -lhdf5-filter-scil -lscil -lscil-util -Wl,--rpath=$INSTALL/lib"
export LT_SYS_LIBRARY_PATH="$PREFIX/lib"

export CC=mpicc

if  [[ ! -e "$PREFIX/lib/libnetcdf.so" ]] ; then
  pushd netcdf*
  #patch -p1 < ../*patch
  
  #./configure --enable-parallel-tests --prefix=$INSTALL
./configure \
 --prefix=${PREFIX} \
 --enable-shared \
 --enable-static \
 --enable-parallel-tests \
 --enable-large-file-tests \
 --enable-pnetcdf 
  #CC=mpicc ./configure --prefix=$INSTALL --enable-parallel
  make -j 4
  make check install
  popd
fi

#gcc  test-netcdf4.c  $CFLAGS -lnetcdf $LDFLAGS   -o test-netcdf4 -Wl,--rpath=$INSTALL/lib
#rm *.nc
#./test-netcdf4
#h5dump -H -p tst_chunks3.nc
