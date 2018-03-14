#!/bin/bash -e
PREFIX=$PWD/../../install/

hdf5=hdf5-1.8.19.tar.gz
if [[ ! -e hdf5-1.8.19.tar.gz ]] ; then
  #wget https://support.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.10.1.tar.gz
  wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8/hdf5-1.8.19/src/$hdf5
fi

tar -xf $hdf5

pushd hdf5-1.8.19
./configure --prefix="${PREFIX}" \
	--enable-hl \
	--enable-shared \
	--enable-static \
	--enable-parallel \
	--enable-build-mode=debug \
	#--enable-fortran \
	CC="mpicc" \
	FC="mpif90" \
	F77="mpi77" \
	CXX="mpic++" \
	CFLAGS="-g -O0" \
	CXXFLAGS="-g -O0"

make -j 4
make -j 4 install

popd
