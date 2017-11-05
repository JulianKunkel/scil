#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

BUILD="$1"
shift
CLEAN=0

ERROR=0

set -- `getopt -u -l "clean" -o "" -- "$@"`
test $# -lt 1  && exit 1
while test $# -gt 0
do
	case "$1" in
		--clean) CLEAN=1;;
		--) ;;
		*) echo "Unknown option $1"; exit 1;;
	esac
	shift
done

if [[ $CLEAN == 1 ]] ; then
   echo "Cleaning"
   rm -rf $BUILD
fi
  if [[ ! -e $BUILD/CMakeCache.txt ]] ; then
    ./configure --build-dir=$BUILD --debug || exit 1
fi
pushd $BUILD > /dev/null
make || exit 1

make test
#if [[ $? != 0 ]] ; then
#  cat Testing/Temporary/LastTest.log
#fi
ERROR=$(($ERROR + $?))
popd  > /dev/null

exit $ERROR
