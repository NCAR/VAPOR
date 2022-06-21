#!/bin/bash

echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
export
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================

mkdir build
cd build

# Since optimizations are disabled, need to disable fortify source otherwise will get barraged with warnings
export CPPFLAGS="`echo $CPPFLAGS|sed 's/-D_FORTIFY_SOURCE=2//g'`"

# Ignore extra warnings
export CPPFLAGS=" \
    $CPPFLAGS \
    "

# Conda does not properly set CMake flags. This should fix it.
export CXXFLAGS="$CPPFLAGS $CXXFLAGS"
export CFLAGS="$CPPFLAGS $CFLAGS"

cmake .. \
    -DCMAKE_INSTALL_PREFIX:PATH="$PREFIX" \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_BUILD_TYPE=DEBUG \
    ${CMAKE_ARGS} \
    -DCMAKE_C_FLAGS_DEBUG="-g -O0" \
    -DCMAKE_CXX_FLAGS_DEBUG="-g -O0" \
    -DPython_EXECUTABLE="$PYTHON"


make -j$(($CPU_COUNT+1))
make install


echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================