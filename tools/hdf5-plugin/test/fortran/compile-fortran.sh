#!/bin/bash

SCIL=/home/julian/wr-git/scil/install/
install=$PWD/install

gfortran -c scil-mod.f95

gcc -c interface-test.c -I$SCIL/include
gfortran -c -fno-underscoring interface.f95 -I$install/include -I$SCIL/include
gfortran interface.o interface-test.o -o interface.exe -L$install/lib/ -lhdf5_fortran -lhdf5 -Wl,--rpath=$install/lib
