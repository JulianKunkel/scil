#!/bin/bash -e

SRC=$(realpath $(dirname $0))

echo "We assume you have installed HDF5 already with the SCIL plugin"
if [[ "$SCIL_INSTALL" == "" ]] ; then
  SCIL_INSTALL=$PWD/../../install/
fi

VERSION=netcdf-4.6.1
if [[ ! -e $SRC/../../deps/$VERSION.tar.gz ]] ; then
  pushd $SRC/../../deps/
  wget ftp://ftp.unidata.ucar.edu/pub/netcdf/$VERSION.tar.gz
  tar -xf $VERSION.tar.gz
  cd $VERSION
  echo "Patching NetCDF for SCIL"
  patch -p1 < $SRC/0001-SCIL-NetCDF4.6.1.patch
  #patch -p1 < $SRC/0001-Patch-for-NetCDF-with-SCIL.patch
  #patch -p1 < $SRC/0002-Flexible-read-information-from-file.patch

  popd
fi

export CFLAGS="-I$SCIL_INSTALL/include -O0 -g3" # -O0 -g3
export CPPFLAGS=$CFLAGS
export LDFLAGS="-L$SCIL_INSTALL/lib -lhdf5 -lhdf5_hl -lhdf5-filter-scil -lscil -lscil-util -Wl,--rpath=$SCIL_INSTALL/lib"
export LT_SYS_LIBRARY_PATH="$SCIL_INSTALL/lib"
export CC=mpicc



#if  [[ ! -e "$SCIL_INSTALL/lib/libnetcdf.so" ]] ; then
  mkdir netcdf4 || true
  pushd netcdf4

  $SRC/../../deps/$VERSION//configure \
    --prefix=${SCIL_INSTALL} \
    --enable-shared \
    --enable-static \
    --enable-parallel-tests \
    --enable-large-file-tests \
    --disable-dap
  make -j 4
  #make check
  make install
  popd
#fi

#gcc  test-netcdf4.c  $CFLAGS -lnetcdf $LDFLAGS   -o test-netcdf4 -Wl,--rpath=$INSTALL/lib
#rm *.nc
#./test-netcdf4
#h5dump -H -p tst_chunks3.nc
