#!/bin/bash

echo "Building dependencies for SCIL from third-party software"

ZFP=zfp-0.5.0
FPZIP=fpzip-1.1.0
WAVELET=wavelet_code

function download(){
		if [[ ! -e $1 ]] ; then
			wget $2/$1
			tar -xf $1
		fi
}

download $FPZIP.tar.gz http://computation.llnl.gov/projects/floating-point-compression/download
download $ZFP.tar.gz http://computation.llnl.gov/projects/floating-point-compression/download
if [[ ! -e $WAVELET.zip ]] ; then
	wget http://eeweb.poly.edu/~onur/wavelet_code.zip
	unzip $WAVELET.zip -d $WAVELET
fi

if [[ ! -e cnoise ]] ; then
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/cnoise.c -P ./cnoise/
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/cnoise.h -P ./cnoise/
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/README.txt -P ./cnoise/
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/Config.mk -P ./cnoise/
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/Makefile -P ./cnoise/

	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/example/test.c -P ./cnoise/test/
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/example/Makefile -P ./cnoise/test/
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/example/test_output.txt -P ./cnoise/test/
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

if [[ ! -e cnoise.a ]] ; then
	echo "  Building cnoise library"
  pushd cnoise/
  make
  popd
	BUILD=1
fi

if [[ $BUILD == 1 ]] ; then
  mkdir -p include/fpzip include/zfp include/cnoise
  cp $FPZIP/inc/* include/fpzip
  cp $ZFP/inc/* include/zfp
	cp cnoise/cnoise.h include/cnoise

  rm *.a
  cp $(find -name "*.a") .
fi

echo "[OK]"
