#!/bin/bash

SCIL=/home/julian/wr-git/scil/install/
install=$PWD/install

gfortran -c scil-mod.f95

mpicc -c interface-test.c -I$SCIL/include 
gfortran -c -fno-underscoring interface.f95 -I$install/include -I$SCIL/include

gfortran interface.o interface-test.o -o interface.exe -L$SCIL/lib -L$install/lib/ -lhdf5_fortran -lhdf5 -lscil -lhdf5-filter-scil -Wl,--rpath=$install/lib
