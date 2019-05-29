#!/bin/bash
export BUILD=/data/build-docker/$HOSTNAME/
export SCIL_INSTALL=$BUILD/install
export HDF5_PLUGIN_PATH=$SCIL_INSTALL/lib

export CFLAGS="-I$SCIL_INSTALL/include -O0 -g3" # -O0 -g3
export CPPFLAGS=$CFLAGS
export LDFLAGS="-L$SCIL_INSTALL/lib -lhdf5 -lhdf5_hl -lhdf5-filter-scil -lscil -lscil-util -Wl,--rpath=$SCIL_INSTALL/lib"
export LT_SYS_LIBRARY_PATH="$SCIL_INSTALL/lib"

cd $BUILD
gcc  /data/tools/netcdf4-patches/test-netcdf4.c  $CFLAGS -lnetcdf $LDFLAGS   -o test-netcdf4 -Wl,--rpath=$SCIL_INSTALL/lib
rm *.nc || true
./test-netcdf4 || true
./install/bin/h5dump -H -p tst_chunks3.nc |grep -C 2 32003
