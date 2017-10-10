#!/bin/bash

PREFIX=$PWD/../../../install/

gfortran -c scil-mod.f90

mpicc -c interface-test.c -I$PREFIX/include

$PREFIX/bin/h5pfc -c interface.f90 -I$PREFIX/include

gfortran interface.o interface-test.o scil-mod.o -o interface.exe -L$PREFIX/lib -lhdf5_fortran -lhdf5 -lscil -lhdf5-filter-scil -Wl,--rpath=$PREFIX/lib
