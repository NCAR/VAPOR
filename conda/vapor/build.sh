#!/bin/sh

# Inputs
[[ -z "$DEBUG_BUILD" ]] && DEBUG_BUILD=true

CMAKE_EXTRA=""

# The env can be either PREFIX or BUILD_PREFIX depending on the build requirements.
# Conda does not configure its variables accordingly.
export ENV="$BUILD_PREFIX"

export PATH="$ENV/bin:$PATH"

# Conda will sometimes set PYTHON to a path that does not exist
export PYTHON="`which python`"

if $DEBUG_BUILD; then
    # If optimizations are disabled, need to disable fortify source otherwise will get barraged with warnings
    export CPPFLAGS="`echo $CPPFLAGS|sed 's/-D_FORTIFY_SOURCE=2//g'`"
    
    CMAKE_EXTRA="$CMAKE_EXTRA -DCMAKE_BUILD_TYPE=Debug"
else
    CMAKE_EXTRA="$CMAKE_EXTRA -DCMAKE_BUILD_TYPE=Release"
fi
CMAKE_EXTRA="$CMAKE_EXTRA -DINSTALLER_OMIT_MAPS=ON"

# Ignore extra warnings
export CPPFLAGS=" \
    $CPPFLAGS \
    -Wno-unused-function \
    -Wno-conversion-null \
    -Wno-deprecated-declarations \
    -Wno-catch-value \
    -Wno-unknown-warning-option \
    -Wno-array-parameter \
    "

# Vapor legacy configuration
export CPPFLAGS="-isystem $ENV/include/freetype2 $CPPFLAGS"

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
# CMAKE_EXTRA="$CMAKE_EXTRA -DCMAKE_FIND_FRAMEWORK=NEVER"
# CMAKE_EXTRA="$CMAKE_EXTRA -DCMAKE_FIND_APPBUNDLE=NEVER"

# When conda messes up the build environment it not only points the python install target to the wrong root,
# it also sometimes points it to the wrong version of python.
SP_DIR="`python -c 'import site; print(site.getsitepackages()[0].replace(\"'$BUILD_PREFIX'\", \"'$PREFIX'\"))'`"
# It also will sometimes decide not to make this dir
mkdir -p "$SP_DIR"

# Our third party libs have a non-standard copy of the GTE library so it is packaged and extracted here.
unzip -d include buildutils/GTE.zip


mkdir -p build
cd build

cmake .. \
    -G 'Unix Makefiles' \
    -DCONDA_BUILD=ON \
    -DBUILD_PYTHON=ON \
    -DBUILD_DOC=ON \
    -DBUILD_UTL=OFF \
    -DBUILD_GUI=OFF \
    -DBUILD_OSP=OFF \
    -DPython_EXECUTABLE="$PYTHON" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    $CMAKE_EXTRA \



make -j$(($CPU_COUNT+1))
make doc
make install

