# Scientific Compression Library (SCIL)
*****

SCIL is available on [GitHub](https://github.com/JulianKunkel/scil).

## Prerequisites

Name/Version 

* gcc
* g++
* lz4
* zlib

The following requirements are automatically downloaded and installed.

* zlib    1.28        [Src](http://www.zlib.net/)
* fpzip   1.1.0      [Src](http://computation.llnl.gov/projects/floating-point-compression)

### Installation prerequisites on Ubuntu

The following command installs all requirements for an Ubuntu 15.10/16.04 System:

		apt-get install g++ gcc liblz4-dev zlib1g-dev libopenjpeg-dev

***************************

## Compilation

+ To adjust the CMAKE based environment to your system run

		./configure [Options]

+ Build the core library

		cd build
		make -j 4

+ Install the core library

		cd build
		make install

+ Build the HDF5 wrapper

		cd tools/hdf5-plugin/
		./configure [Options]
		make install

+ You have to copy the HDF5 wrapper to the plugin install directory of HDF5!
