#!/bin/bash

echo "Building dependencies for SCIL from third-party software"

ZFP=zfp-0.5.0
FPZIP=fpzip-1.1.0

if [[ ! -e ${FPZIP}.tar.gz ]] ; then
	wget http://computation.llnl.gov/projects/floating-point-compression/download/$FPZIP.tar.gz
	wget http://computation.llnl.gov/projects/floating-point-compression/download/$ZFP.tar.gz
	for x in *.gz ; do
		tar -xf $x
	done
fi

BUILD=0

if [[ ! -e libzfp.a ]] ; then
	echo "  Building fpzip shared library"
	pushd $ZFP
	cp ../config-zfp Config
	make shared
	make
	popd
	BUILD=1
fi

if [[ ! -e libfpzip.a ]] ; then
  echo "  Building zfp shared library"
  pushd $FPZIP/src
  make -f ../../Makefile-fpzip-1.1.0
  popd
	BUILD=1
fi

if [[ $BUILD == 1 ]] ; then
  mkdir -p include/fpzip include/zfp
  cp $FPZIP/inc/* include/fpzip
  cp $ZFP/inc/* include/zfp

  rm *.a
  cp $(find -name "*.a") .
fi

echo "[OK]"
