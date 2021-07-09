#!/bin/bash

LIB="`otool -L "$1" | grep -F libomp | xargs | cut -d\  -f1`"
install_name_tool -change "$LIB" "@rpath/libomp.dylib" "$1"
