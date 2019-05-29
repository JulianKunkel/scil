# Docker Tests
This directory contains Dockerfiles to build SCIL in different distributions.

To setup all containers:
$ ./prepare.sh

To run all tests:
./run-all-tests.sh

# To setup an environment with all software packages

A Dockerfile with NetCDF and HDF5 is created as follows:
$ docker build -t kunkel/scil:ubuntu18.04-hdf5-nc ubuntu18.04-hdf5-nc

Note that the following script builds the core software from scratch and in the order:
  * SCIL
  * HDF5
  * SCIL-HDF5 plugin
  * NetCDF with SCIL patch

To start the build process run:
$ docker run -it --rm -u $(id -u):$(id -g) -v $PWD/../:/data/ -h ubuntu18.04-hdf5-nc kunkel/scil:ubuntu18.04-hdf5-nc /data/test-docker/ubuntu18.04-hdf5-nc/test-script.sh

This connects the current software and mounts it into /data inside the container.

You can use the environment to test the software, after the build, run:
$ docker run -it --rm -u $(id -u):$(id -g) -v $PWD/../:/data/ -h ubuntu18.04-hdf5-nc kunkel/scil:ubuntu18.04-hdf5-nc /bin/bash

An example application using NetCDF4 is build against this toolchain by running:
$ /data/test-docker/ubuntu18.04-hdf5-nc/run-example.sh

This should output:

      FILTERS {
         USER_DEFINED_FILTER {
            FILTER_ID 32003
            COMMENT SCIL
            PARAMS { -1210634288 22009 3 0 4 0 4 0 4 0 0 0 0 0 1 0 }
