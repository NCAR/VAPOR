#!/bin/sh

# Inputs
[[ -z "$DEBUG" ]] && DEBUG=false

CMAKE_EXTRA=""

# The env can be either PREFIX or BUILD_PREFIX depending on the build requirements.
# Conda does not configure its variables accordingly.
export ENV="$BUILD_PREFIX"
export PATH="$ENV/bin:$PATH"

if $DEBUG; then
    # If optimizations are disabled, need to disable fortify source otherwise will get barraged with warnings
    export CPPFLAGS="`echo $CPPFLAGS|sed 's/-D_FORTIFY_SOURCE=2//g'`"
    CMAKE_EXTRA="$CMAKE_EXTRA -DCMAKE_BUILD_TYPE=Debug"
else
    CMAKE_EXTRA="$CMAKE_EXTRA -DCMAKE_BUILD_TYPE=Release"
fi

export CPPFLAGS="$CPPFLAGS -isystem $ENV/include"

# cmake ignores CPPFLAGS
export CXXFLAGS="$CXXFLAGS $CPPFLAGS"
export CFLAGS="$CFLAGS $CPPFLAGS"

# Conda will sometimes set these to invalid values which end up breaking the build
unset CMAKE_ARGS
unset CMAKE_PREFIX_PATH

# Prevent linking outside libraries
export CMAKE_LIBRARY_PATH="$PREFIX/lib:$BUILD_PREFIX/lib"
export CMAKE_PREFIX_PATH="$PREFIX:$BUILD_PREFIX"

mkdir build
cd build

cmake .. \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    $CMAKE_EXTRA \

make -j$(($CPU_COUNT+1))
make install

