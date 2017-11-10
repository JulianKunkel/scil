#!/bin/bash
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
pushd $SCRIPTPATH >/dev/null
if [[ $1 == "clean" ]] ; then
  echo "rm -f allquant-copy-of-*.c"
  rm -f allquant-copy-of-*.c
else

for file in $(grep -rl --exclude=*.sh --exclude=allquant-copy-of-*.c -e 'hints.force_compression_methods = "1"' -e 'hints.force_compression_methods = "3"')
do
  sed 's/hints.force_compression_methods = "1"/hints.force_compression_methods = "12"/' "$file" | sed 's/hints.force_compression_methods = "3"/hints.force_compression_methods = "12"/' >"allquant-copy-of-$file"
  echo "allquant-copy-of-$file"
done

fi
popd >/dev/null
