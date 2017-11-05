#!/bin/bash

cd $(dirname "$0")
# ARGS=" /bin/bash $@"
ARGS=" /data/test-docker/run-machine-test.sh $@"
OPT="-it --rm -u $(id -u):$(id -g) -v $PWD/../:/data/"
ERROR=0
docker run $OPT -h ubuntu14.04 kunkel/scil:ubuntu14.04 $ARGS
ERROR=$(($ERROR+$?))
docker run $OPT -h ubuntu16.04 kunkel/scil:ubuntu16.04 $ARGS
ERROR=$(($ERROR+$?))
docker run $OPT -h ubuntu17.04 kunkel/scil:ubuntu17.04 $ARGS
ERROR=$(($ERROR+$?))
docker run $OPT -h centos6 kunkel/scil:centos6 $ARGS
ERROR=$(($ERROR+$?))
docker run $OPT -h centos7 kunkel/scil:centos7 $ARGS
ERROR=$(($ERROR+$?))
docker run $OPT -h archlinux kunkel/scil:archlinux $ARGS
ERROR=$(($ERROR+$?))

if [[ $ERROR != 0 ]] ; then
	echo "Errors occured: $ERROR"
else
	echo "OK all tests passed!"
fi
