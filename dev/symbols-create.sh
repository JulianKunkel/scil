#!/bin/bash -e

echo "Rebuild after setting:"
cat << EOF > src/symbols.map
{
 global:*;
};
EOF

echo "Now make clean, rebuild, then press enter!"
pushd build
make clean
make -j
popd

(
for F in $(find build -name "*.so" |grep -v deps) ; do
nm --defined-only $F
done
) |cut -b "20-" | grep scil > src/symbols.txt

(
echo "{"
echo " global:"
for s in $(cat src/symbols.txt) ; do
  echo "$s;"
done
echo "  local:*;"
echo "};"
) > src/symbols.map


echo "Complete! Now make clean and rebuild"
pushd build
make clean
make -j
popd
