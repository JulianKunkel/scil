#!/bin/bash

echo "Running test for $HOSTNAME"
BUILD=/data/build-docker/$HOSTNAME/
export SCIL_INSTALL=$BUILD/install

mkdir -p $BUILD

CLEAN=0
ERROR=0

set -- `getopt -u -l "clean" -o "" -- "$@"`
test $# -lt 1  && exit 1
while test $# -gt 0
do
	case "$1" in
		--clean) CLEAN=1;;
		--) ;;
		*) echo "Unknown option $1"; exit 1;;
	esac
	shift
done

if [[ $CLEAN == 1 ]] ; then
   echo "Cleaning"
   rm -rf $BUILD
fi
  if [[ ! -e $BUILD/CMakeCache.txt ]] ; then
    ./configure --build-dir=$BUILD --prefix=$BUILD/install --debug || exit 1
fi
pushd $BUILD > /dev/null
make || exit 1

make test
#if [[ $? != 0 ]] ; then
#  cat Testing/Temporary/LastTest.log
#fi
ERROR=$(($ERROR + $?))

make install

# Install HDF5 and NetCDF on top of SCIL:
/data/tools/hdf5-plugin/install-hdf5.sh

echo "Install SCIL plugin for HDF5"
pushd ../../tools/hdf5-plugin/
./configure --prefix=$SCIL_INSTALL --build-dir=$BUILD/hdf5-plugin --with-scil=$SCIL_INSTALL --with-hdf5=$SCIL_INSTALL --with-cc=$(which gcc) --rpath=$SCIL_INSTALL/lib
popd

pushd $BUILD/hdf5-plugin
make -j 4
make -j 4 install
popd

cd $BUILD
/data/tools/netcdf4-patches/install.sh

echo "All prepared and installed in $SCIL_INSTALL, see /data/test-docker/ubuntu18.04-hdf5-nc/run-example.sh"

if [[ $ERROR != 0 ]]  ; then
  echo "Errors occured, see: "
  find $BUILD/ -name LastTest.log | sed "s#/data/#../../#"
  exit 1
fi

exit $ERROR
