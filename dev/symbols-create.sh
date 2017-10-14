#!/bin/bash

(
nm build/util/libscil-util.so
nm build/compression/libscil.so
nm build/pattern/libscil-patterns.so
) |cut -b "20-" |grep scil > src/symbols.txt
