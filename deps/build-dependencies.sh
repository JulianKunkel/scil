#!/bin/bash

echo "Building dependencies for SCIL from third-party software"

ZFP=zfp-0.5.0
FPZIP=fpzip-1.1.0

function download(){
		if [[ ! -e $1 ]] ; then
			wget $2/$1
			tar -xf $1
		fi
}

download $FPZIP.tar.gz http://computation.llnl.gov/projects/floating-point-compression/download
download $ZFP.tar.gz http://computation.llnl.gov/projects/floating-point-compression/download

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
