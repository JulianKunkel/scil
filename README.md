# Scientific Compression Library (SCIL) [![Build Status](https://travis-ci.org/JulianKunkel/scil.svg?branch=master)](https://travis-ci.org/JulianKunkel/scil) [![Codecov Status](https://codecov.io/github/JulianKunkel/scil/coverage.svg?branch=master)
*****

SCIL is available on [GitHub](https://github.com/JulianKunkel/scil).

## Prerequisites

Detailed information how to install required software packages are provided in the directory: 
  * test/docker/<distribution>/Dockerfile

When running configure, the system will automatically install additional requirements.

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
