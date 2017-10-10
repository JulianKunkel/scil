#!/bin/bash

PREFIX=$PWD/../../../install/

gfortran -c scil-mod.f95

mpicc -c interface-test.c -I$PREFIX/include
gfortran -c -fno-underscoring interface.f95 -I$PREFIX/include

gfortran interface.o interface-test.o -o interface.exe -L$PREFIX/lib -lhdf5_fortran -lhdf5 -lscil -lhdf5-filter-scil -Wl,--rpath=$PREFIX/lib
