# Installation of a development environment
This file describes how to install a SCIL development environment including NetCDF and HDF5 support.

In the following guide, we use the directory as set in the variable SCIL_INSTALL to define where we want to install SCIL.

Note that this builds HDF5, NetCDF and SCIL with debug options, thus performance is not optimal!

We assume you are in the root directory of SCIL.

## Dependencies

$ apt-get install libgsl-dev

## Step by step

First, set the variable (example with bash):
$ export SCIL_INSTALL=$PWD/install

We install SCIL with the optional system NetCDF support. NetCDF support is only needed for building tools to conduct tests / compress NetCDF files.

$ ./configure --prefix=$SCIL_INSTALL --debug
$ cd build
$ make -j 4
$ make install

Installation of HDF5 (without Fortran support):
$ ./tools/hdf5-plugin/install-hdf5.sh

Installation of the SCIL HDF5 Plugin:
$ pushd ./tools/hdf5-plugin
$ ./configure --with-hdf5=$SCIL_INSTALL --with-scil=$SCIL_INSTALL --prefix=$SCIL_INSTALL
$ cd build; make -j 4 && make install
$ popd

Installation of the NetCDF4 with SCIL support:
$ ./tools/netcdf4-patches/install.sh

## Testing

$ gcc -I$SCIL_INSTALL/include -L$SCIL_INSTALL/lib tools/netcdf4-patches/test-netcdf4.c -lnetcdf -lhdf5  -lscil -l scil-util -Wl,-rpath,$SCIL_INSTALL/lib -o test-netcdf-scil

That shouldn't be needed: export HDF5_PLUGIN_PATH=$SCIL_INSTALL/lib/
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SCIL_INSTALL/lib
$ ./test-netcdf-scil
This should show some data:
$ $SCIL_INSTALL/bin/h5dump -p tst_chunks3.nc | grep -A 30 '"var_compressed"'
