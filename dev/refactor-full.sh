#!/bin/bash

echo "This script prepares cscout to perform refactoring"
echo "Note: this is broken..."

export CSCOUT_HOME=$PWD/cscout/install

if [[ ! -e ./cscout/src/build/cscout ]] ; then
 git clone https://github.com/dspinellis/cscout.git
 pushd cscout
 make -j 4
 make install INSTALL_PREFIX=$CSCOUT_HOME
 popd
fi

(
echo 'workspace scil {'
echo 'ro_prefix "/usr/include"'
echo 'ro_prefix "/usr/include/hdf5/serial/"'
echo "project scil {"
echo "file " $(find ../src/ -name "*.c") $(find ../tools/ -name "*.c")
echo	"}"
echo "}"
) > proj.csw

$CSCOUT_HOME/bin/cswc proj.csw > proj.cs

$CSCOUT_HOME/bin/cscout proj.cs
