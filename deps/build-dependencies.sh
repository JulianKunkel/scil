#!/bin/bash -e

SRC="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TGT=$PWD/deps

DOWNLOAD_ONLY=0

if [[ "$1" == "download" ]] ; then
DOWNLOAD_ONLY=1
fi

mkdir -p "$TGT"
cd "$TGT"
echo "Building dependencies for SCIL from third-party software"

function download(){
		if [[ ! -e "$SRC/$1" ]] ; then
			wget "$2/$1" -O "$SRC/$1"
		fi
		if [[ $DOWNLOAD_ONLY == 1 ]] ; then
			return
		fi

		if [[ ! -e "$TGT/$1" ]] ; then
			tar -xf "$SRC/$1"
		fi
}

ZFP=zfp-0.5.0
FPZIP=fpzip-1.1.0
WAVELET=wavelet_code

download $FPZIP.tar.gz http://computation.llnl.gov/projects/floating-point-compression/download
download $ZFP.tar.gz http://computation.llnl.gov/projects/floating-point-compression/download

if [[ ! -e "$SRC/$WAVELET.zip" ]] ; then
	wget http://eeweb.poly.edu/~onur/$WAVELET.zip -O "$SRC/$WAVELET.zip"
fi

if [[ ! -e "$TGT/$WAVELET"&& $DOWNLOAD_ONLY == 0 ]] ; then
	unzip "$SRC/$WAVELET.zip" -d "$TGT/$WAVELET"
	if [[ $? != 0 ]] ; then
		echo "Error unzip $SRC/$1"
		exit 1
	fi
fi

if [[ ! -e "$SRC/cnoise/test/test_output.txt" ]] ; then
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/cnoise.c -P "$SRC/cnoise/"
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/cnoise.h -P "$SRC/cnoise/"
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/README.txt -P "$SRC/cnoise/"
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/Config.mk -P "$SRC/cnoise/"
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/Makefile -P "$SRC/cnoise/"

	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/example/test.c -P "$SRC/cnoise/test/"
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/example/Makefile -P "$SRC/cnoise/test/"
	wget https://people.sc.fsu.edu/~jburkardt/c_src/cnoise/example/test_output.txt -P "$SRC/cnoise/test/"
fi

if [[ ! -e "$SRC/SZ" ]] ; then
	pushd "$SRC"
	git clone https://github.com/disheng222/SZ.git
	# problems with unstable most recent version, freezed to stable version
	# TODO: check for stable versions on larger timescale cycles
	cd SZ
	git reset --hard 85d43e0920f87082f97f270619b977fd16d8e3bd
	popd
fi

if [[ ! -e "$SRC/CubismZ" ]] ; then
	pushd "$SRC"
	git clone https://github.com/cselab/CubismZ.git
	popd
fi

if [[ ! -e "$SRC/lz4" ]] ; then
	pushd "$SRC"
	git clone https://github.com/lz4/lz4.git
	popd
fi

if [[ $DOWNLOAD_ONLY == 1 ]] ; then
  exit 0
fi


if [[ ! -e "$TGT/cnoise/" ]] ; then
	cp -r "$SRC/cnoise/" .
fi


BUILD=0

if [[ ! -e libzfp.a ]] ; then
	echo "  Building fpzip shared library"
	pushd "$ZFP" > /dev/null
	cp "$SRC/config-zfp" Config
	make shared
	make
	popd > /dev/null
	BUILD=1
fi

if [[ ! -e libfpzip.a ]] ; then
  echo "  Building zfp shared library"
  pushd "$FPZIP/src" > /dev/null
  make -f "$SRC/Makefile-fpzip-1.1.0"
  popd > /dev/null
	BUILD=1
fi


if [[ ! -e libcnoise.a ]] ; then
	echo "  Building cnoise library"

  pushd cnoise/ > /dev/null
  make
  popd > /dev/null
	BUILD=1
fi

if [[ ! -e liblz4.a ]] ; then
	mkdir -p include/lz4 || true
	pushd "$SRC/lz4/" > /dev/null
	make clean || true
	make -j 4 CFLAGS="-fPIC"
	popd > /dev/null
	BUILD=1
fi

if [[ ! -e libsz.a ]] ; then
	echo "  Building SZ"
	mkdir SZ || true
	pushd SZ > /dev/null
	CFLAGS="-I\"$SRC/SZ/sz/include\" -I\"$SRC/SZ/zlib\" -fPIC -O3"  "$SRC/SZ/configure"  --disable-shared --prefix="$PWD/install"
	make -j install
	popd > /dev/null
	BUILD=1
fi

if [[ false ]] ; then
	mkdir CubismZ || true
fi

if [[ $BUILD == 1 ]] ; then
  mkdir -p include/fpzip include/zfp include/cnoise include/sz
  cp $FPZIP/inc/* include/fpzip
  cp $ZFP/inc/* include/zfp
  cp cnoise/cnoise.h include/cnoise
  cp -r ./SZ/install/include/* include/sz

  rm *.a || true # ignore error
  cp $SRC/lz4/lib/lz4.h include/lz4/
  cp ./SZ/install/lib/libzlib.a ./SZ/install/lib/libsz.a ./zfp-0.5.0/lib/libzfp.a ./cnoise/libcnoise.a ./fpzip-1.1.0/lib/libfpzip.a .
  cp $SRC/lz4/lib/liblz4.a .
  echo "[OK]"
else
  echo "[Already built]"
fi
