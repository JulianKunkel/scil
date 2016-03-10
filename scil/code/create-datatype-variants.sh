#!/bin/bash

DATATYPES="float double"

TMP=tmp.x

if [[ "$1" == "" ]];
then
  DIR=$PWD
else
  DIR=$1
fi


rm $TMP 2>/dev/null

for f in $(find $DIR -name "*.dtype"); do
  f=${f#$DIR/}
  target=${f%%.dtype}
  echo "Processing $f"

  mkdir -p $(dirname $target)

  for t in $DATATYPES; do
    sed "s/<DATATYPE>/$t/g" $DIR/$f >> $TMP
    echo >> $TMP
  done
  mv $TMP $target

done
