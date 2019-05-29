#!/bin/bash -e
SRC=$(realpath $(dirname $0))

if [[ "$SCIL_INSTALL" == "" ]] ; then
  SCIL_INSTALL=$PWD/../../install/
fi

hdf5=hdf5-1.10.5
if [[ ! -e $SRC/../../deps/$hdf5.tar.bz2 ]] ; then
  pushd $SRC/../../deps/
  wget https://support.hdfgroup.org/ftp/HDF5/current/src/$hdf5.tar.bz2
  tar -xf $hdf5.tar.bz2
  popd
fi

mkdir hdf5 || true
pushd hdf5

$SRC/../../deps/$hdf5/configure --prefix="${SCIL_INSTALL}" \
	--enable-hl \
	--enable-shared \
	--enable-static \
	--enable-parallel \
	--enable-build-mode=debug \
  --with-default-plugindir="${SCIL_INSTALL}/lib/" \
	#--enable-fortran \
	CC="mpicc" \
	FC="mpif90" \
	F77="mpi77" \
	CXX="mpic++" \
	CFLAGS="-g3 -O0" \
	CXXFLAGS="-g3 -O0"

make -j 4
make -j 4 install

popd
