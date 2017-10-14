# Scientific Compression Library (SCIL) [![Build Status](https://travis-ci.org/JulianKunkel/scil.svg?branch=master)](https://travis-ci.org/JulianKunkel/scil) ![Codecov Status](https://codecov.io/github/JulianKunkel/scil/coverage.svg?branch=master)
*****

SCIL is available on [GitHub](https://github.com/JulianKunkel/scil).

## Prerequisites

Detailed information how to install required software packages are provided in the directory:
  * test/docker/<distribution>/Dockerfile
The commands provided after the RUN instruction are those necessary to install prerequisites.

Once these are installed use, e.g., ./configure --prefix=$PWD/install
Note that SCIL requires to be built out of tree, meaning one may not use "cmake ./" but instead call cmake from an empty directory.

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

## Directory structure

* dev: contains tools for development
* deps: contains tools to automatically setup dependencies
* doc: contains some documentations (more when you run make)
* src: the source code of the core libraries provided, see below for details
* test-docker: contains scripts to setup a docker test environment for various distributions
* tools: additional tools using SCIL

### ./src -- the source directory

The source directory is split into several components:

* compression: the compression library
  * algo: all compression algorithms
    * util: utilities for the individual algorithms
* core: core datatypes and functions that are accessible by users
* pattern: library for several synthetic patterns
* test: unit and system-integration test code for any component
* tools: command line tools
  * util: utility functions for command line tools
    * file-formats: file formats library for tools
* util: supportive functions that may be used by any other library and function but
        usually NOT by end-users. Util shall not have any dependency to any external component except to core.
