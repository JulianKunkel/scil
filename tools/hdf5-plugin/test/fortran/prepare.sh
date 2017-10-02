#!/bin/bash -e
if [[ ! -e hdf5-1.10.1.tar.gz ]] ; then
  wget https://support.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.10.1.tar.gz  
fi

tar -cf hdf5-1.10.1.tar.gz

PREFIX=$PWD/install

pushd hdf5-1.10.1
./configure --prefix="${PREFIX}" \
	--enable-hl \
	--enable-shared \
	--enable-static \
	--enable-parallel \
	--enable-build-mode=debug \
	--enable-fortran \
	--enable-fortran2003 \
	CC="mpicc" \
	FC="mpif90" \
	F77="mpi77" \
	CXX="mpic++" \
	CFLAGS="-g -O0" \
	CXXFLAGS="-g -O0"

make -j install

popd

