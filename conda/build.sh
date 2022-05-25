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


# Our third party libs have a non-standard copy of the GTE library so it is packaged and extracted here.
unzip -d include buildutils/GTE.zip


mkdir build
cd build

# Vapor legacy configuration
export CPPFLAGS="-isystem $PREFIX/include/freetype2 $CPPFLAGS"

# Since optimizations are disabled, need to disable fortify source otherwise will get barraged with warnings
export CPPFLAGS="`echo $CPPFLAGS|sed 's/-D_FORTIFY_SOURCE=2//g'`"

# Ignore extra warnings
export CPPFLAGS=" \
    $CPPFLAGS \
    -Wno-unused-function \
    -Wno-conversion-null \
    -Wno-deprecated-declarations \
    -Wno-catch-value \
    "

# Conda does not properly set CMake flags. This should fix it.
export CXXFLAGS="$CPPFLAGS $CXXFLAGS"
export CFLAGS="$CPPFLAGS $CFLAGS"

cmake .. \
    -DBUILD_PYTHON=ON \
    -DBUILD_GUI=OFF \
    -DBUILD_UTL=ON \
    -DBUILD_DOC=ON \
    -DBUILD_OSP=OFF \
    -DCONDA_BUILD=ON \
    -DCMAKE_INSTALL_PREFIX:PATH="$PREFIX" \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_BUILD_TYPE=DEBUG \
    -DCMAKE_C_FLAGS_DEBUG="-g -O0" \
    -DCMAKE_CXX_FLAGS_DEBUG="-g -O0" \
    -DPython_EXECUTABLE="$PYTHON" \
    ${CMAKE_ARGS}


# Show build command
make -j$(($CPU_COUNT+1)) VERBOSE=1 common

# make -j$((nproc`+1))
make -j$(($CPU_COUNT+1))

make doc
make install


# Copy python to SP_DIR


echo pwd=`pwd`
echo pwdsed=`pwd|sed 's/\//_/g'`
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
echo build =======================================================================================
