#!/bin/bash

install=$PWD/install

gfortran -c scil-mod.f95

gcc -c interface-test.c
gfortran -c -fno-underscoring interface.f95 -I$install/include
gfortran interface.o interface-test.o -o interface.exe -L$install/lib/ -lhdf5_fortran -lhdf5 -Wl,--rpath=$install/lib
