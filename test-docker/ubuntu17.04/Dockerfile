FROM ubuntu:17.04

WORKDIR /data
RUN apt-get update
RUN apt-get install -y libopenmpi-dev openmpi-bin mpich git cmake pkg-config gcc-6 nano
RUN apt-get update && apt-get install -y libgsl-dev unzip liblz4-dev python3 zlib1g-dev python
