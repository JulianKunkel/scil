#!/bin/bash

# ARGS=" /bin/bash $@"
ARGS=" /data/test/docker/run-machine-test.sh $@"
OPT="-it --rm -v $PWD/../../:/data/"
ERROR=0
# While we would like to test on all systems it really slows the travis build down
# Travis does not cache any docker images so they have to be rebuild each time
# That's why we only run one test on travis for now
docker run $OPT -h ubuntu14.04 kunkel/scil:ubuntu14.04 $ARGS
ERROR=$(($ERROR+$?))
# docker run $OPT -h ubuntu16.04 kunkel/scil:ubuntu16.04 $ARGS
# ERROR=$(($ERROR+$?))
# docker run $OPT -h ubuntu17.04 kunkel/scil:ubuntu17.04 $ARGS
# ERROR=$(($ERROR+$?))
# docker run $OPT -h centos6 kunkel/scil:centos6 $ARGS
# ERROR=$(($ERROR+$?))
# docker run $OPT -h centos7 kunkel/scil:centos7 $ARGS
# ERROR=$(($ERROR+$?))
# docker run $OPT -h archlinux kunkel/scil:archlinux $ARGS
# ERROR=$(($ERROR+$?))

if [[ $ERROR != 0 ]] ; then
	echo "Errors occured: $ERROR"
    exit 1
else
	echo "OK all tests passed!"
    exit 0
fi
