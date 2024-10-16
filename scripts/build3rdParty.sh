#!/bin/bash

# To install:           Uncomment 'prerequisites' and the library name you'd like, 
#                       listed at the bottom of this file.  Then run build.sh.
#
# To uninstall:         Re-install your target library, then run the following
#                       command from the build directory.
#
#                       xargs rm < install_manifest.txt

set -xe

while getopts "o:b:" flag; do
    case "${flag}" in
        o) OS=${OPTARG};;
        b) baseDir=${OPTARG};;
    esac
done

if [ -z "$OS" ]; then
    echo "Error: -o flag is required to specify the target operating system [macOSx86, appleSilicon, Ubuntu]"
    exit 1
elif [ -z "$baseDir" ]; then
    echo "No -b (base directory) option given.  Defaulting to /usr/local/VAPOR-Deps"
    baseDir='/usr/local/VAPOR-Deps'
fi
#baseDir='/glade/campaign/cisl/vast/vapor/third-party'

srcDir="$baseDir/2024-Sept-src"
archiveName="2024-Sept-${OS}"
installDir="$baseDir/current"

echo OS ${OS}
echo baseDir $baseDir
echo srcDir $srcDir
echo installDir $installDir

macOSMinVersion=""
shopt -s expand_aliases
alias make='make -j8'
alias configure='./configure'
alias config='./config'
if [[ "$OS" == "macOSx86" ]]; then
    alias configure='./configure --host=x86_64-apple-darwin'
    alias config='./config darwin64-x86_64-cc'
    alias brew='arch -x86_64 /usr/local/bin/brew'
    export PATH="/usr/local/bin:$PATH"
    macOSMinVersion="10.15.0"
elif [[ "$OS" == "appleSilicon" ]]; then
    macOSMinVersion="12.0.0"
    alias brew='/opt/homebrew/bin/brew'
    export PATH="/opt/homebrew/bin:$PATH"
fi

#echo "Homebrew alias:"
#alias brew
#echo "PATH ${PATH}"

macOSx86Prerequisites() {
    brew uninstall python@3.9 || true
    export CMAKE_OSX_ARCHITECTURES=x86_64
    export CFLAGS="-arch x86_64"
    export LDFLAGS="-arch x86_64"
    macOSPrerequisites
}

macOSPrerequisites() {
    export MACOSX_DEPLOYMENT_TARGET=$macOSMinVersion
    export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)

    export CC="/opt/local/bin/clang"
    export CXX="/opt/local/bin/clang++"
    export CFLAGS="$CFLAGS -isysroot $(xcrun --sdk macosx --show-sdk-path)"
    export LDFLAGS="$LDFLAGS -isysroot $(xcrun --sdk macosx --show-sdk-path)"
    export CXXFLAGS="$CFLAGS"
    export CPPFLAGS="$CFLAGS"

    brew doctor
    brew cleanup
    brew install cmake autoconf atool libtool automake
    brew install libxml2 xz
    brew install pkg-config gdbm tcl-tk 
    brew install gettext
    brew uninstall --ignore-dependencies gettext

    #port install clang-17 +universal
    #sudo port select --set clang mp-clang-17
    #xcode-select --install
}

ubuntuPrerequisites() {
    CC='gcc'
    CXX='g++'
    LDFLAGS="-L$installDir/lib"
    CFLAGS="-I$installDir/include"
    export DEBIAN_FRONTEND=noninteractive
    apt update -y
    apt upgrade -y

    apt install -y \
        rsync \
        build-essential \
        libgl1-mesa-dev \
        qtbase5-dev \
        libicu-dev \
        m4 \
        libcurl4-openssl-dev \
        libxau-dev \
        autoconf \
        libtool \
        libxcb-xinerama0 \
        libxcb-xinerama0-dev \
        pkg-config \
        unzip \
        libssl-dev
    
    # Qt
    apt-get install -y \
        '^libxcb.*-dev' \
        libx11-xcb-dev \
        freeglut3-dev \
        libglu1-mesa-dev \
        libxrender-dev \
        libxi-dev \
        libxkbcommon-dev \
        libxkbcommon-x11-dev

    wget https://github.com/Kitware/CMake/releases/download/v3.30.5/cmake-3.30.5.tar.gz
    tar -xzvf cmake-3.30.5.tar.gz
    cd cmake-3.30.5
    ./bootstrap
    make -j$(nproc)
    make install

    # scipy
    apt-get install -y libffi-dev
}

windowsPrerequisites() {
    CC='i686-w64-mingw-gcc'
    CXX='i686-w64-mingw-g++'
    apt update -y
    apt upgrade -y

    # all for cmake
    apt-get update
    apt-get install -y gpg wget
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null
    DEBIAN_FRONTEND=noninteractive apt install -y software-properties-common
    apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
    apt install -y cmake --allow-unauthenticated

    apt install mingw-64

    #choco install visualstudio2019-workload-vctools python cmake -y
    #setx /M PATH "%PATH%;C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin"
    #python -m pip install gdown
}

libpng() {
    cd $srcDir
    local library='libpng-1.6.39'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz 
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build

    args=(
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_LIBDIR=lib
    )
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "appleSilicon" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    fi
    if [ "$OS" == "macOSx86" ]; then 
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi

    cmake "${args[@]}" ..
    make && make install
}

assimp() {
    cd $srcDir
    local library='assimp-5.2.5' #requires c++17 compiler
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build

    args=(
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_CXX_FLAGS="-O3 -Wno-error=deprecated-declarations"
        -DASSIMP_BUILD_TESTS=OFF
    )
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "appleSilicon" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    fi
    if [ "$OS" == "macOSx86" ]; then 
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi

    cmake "${args[@]}" ..
    make && make install
}

zlib() {
    cd $srcDir
    local library='zlib-1.2.13'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build

    args=(
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DCMAKE_BUILD_TYPE=Release 
    )
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "appleSilicon" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    fi
    if [ "$OS" == "macOSx86" ]; then 
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi


    cmake "${args[@]}" ..
    make && make install
}

#Note: After configuration, we need to make sure both zlib and szlib are enabled.
szip() {
    cd $srcDir
    local library='szip-2.1.1'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    cd $srcDir/$library
    args=(
        --prefix=$installDir
    )
    ./configure "${args[@]}"

    make && make install
}

#hdfVersion='1.14.0'
#hdfVersion='1.13.3'
hdfVersion='1.12.2'
hdf5() {
    cd $srcDir
    if [ "$OS" == "macOSx86" ]; then
        tar xvf hdf5/hdf5-$hdfVersion-Std-macos11_64-clang.tar.gz && cd hdf
        ./HDF5-$hdfVersion-Darwin.sh --prefix=$installDir --exclude-subdir --skip-license
    elif [ "$OS" == "appleSilicon" ]; then
        tar xvf hdf5/hdf5-$hdfVersion-Std-macos11m1_64-clang.tar.gz && cd hdf
        ./HDF5-$hdfVersion-Darwin.sh --prefix=$installDir --exclude-subdir --skip-license
    else
        tar xvf hdf5/hdf5-$hdfVersion-Std-centos7_64-7.2.0.tar.gz && cd hdf
        ./HDF5-$hdfVersion-Linux.sh --prefix=$installDir --exclude-subdir --skip-license
    fi

    ln -fs $installDir/HDF_Group/HDF5/$hdfVersion/lib/plugin/ $installDir/share/plugins
}

hdf5src() {
    cd $srcDir
    local library="hdf5-${hdfVersion}"
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    export CMAKE_INCLUDE_PATH=/path/to/libsz/include
    export CMAKE_LIBRARY_PATH=/path/to/libsz/lib

    args=(
    -DCMAKE_INSTALL_PREFIX=$installDir
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_RPATH=$installDir
    -DCMAKE_PREFIX_PATH=$installDir
    -DZLIB_DIR=$installDir
    -DUSE_SHARED_LIBS:BOOL=ON
    -DBUILD_SHARED_LIBS:BOOL=ON
    -DHDF5_ENABLE_RPATH:BOOL=ON
    -DHDF5_BUILD_WITH_INSTALL_NAME:BOOL=ON
    -DHDF5_ENABLE_THREADSAFE:BOOL=ON
    -DHDF5_ENABLE_SZIP_SUPPORT:BOOL=ON
    -DHDF5_ENABLE_Z_LIB_SUPPORT:BOOL=ON
    -DHDF5_BUILD_HL_LIB:BOOL=ON
    -DHDF5_BUILD_TOOLS:BOOL=ON
    -DALLOW_UNSUPPORTED:BOOL=ON
    )
    if [ "$OS" == "macOSx86" ]; then 
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi
    cmake "${args[@]}" ..
    make
    make install
}

zstd() {
    cd $srcDir
    local library='zstd-1.5.6'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar
    mkdir -p $srcDir/$library/build/cmake/build && cd $srcDir/$library/build/cmake/build

    args=(
        -DCMAKE_INSTALL_PREFIX=$installDir
    )

    if [ "$OS" == "macOSx86" ]; then 
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi
    if [ "$OS" == "appleSilicon" ]; then 
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
        args+=(-DCMAKE_OSX_ARCHITECTURES=arm64)
    fi

    cmake "${args[@]}" ..
    make && make install
}

netcdf() {
    cd $srcDir
    local library='netcdf-c-4.9.1'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build

    args=(
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DCMAKE_PREFIX_PATH=$installDir/HDF_Group/HDF5/$hdfVersion
        -DHDF5_DIR=$installDir/HDF_Group/HDF5/$hdfVersion
        -DHDF5_ROOT=$installDir/HDF_Group/HDF5/$hdfVersion
        -DCMAKE_INSTALL_LIBDIR=lib
        -DENABLE_BYTERANGE=False
        -DENABLE_DAP=False
        -DCMAKE_BUILD_TYPE=Release
    )
    if [ "$OS" == "macOSx86" ]; then 
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi
    if [ "$OS" == "appleSilicon" ]; then 
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
        args+=(-DCMAKE_OSX_ARCHITECTURES=arm64)
    fi

    export HDF5_ROOT=$installDir/HDF_Group/HDF5/$hdfVersion
    export LD_LIBRARY_PATH=$HDF5_ROOT/lib
    export CPPFLAGS=-I$HDF5_ROOT/include
    export LDFLAGS=-L$HDF5_ROOT/lib
    cmake "${args[@]}" ..
    make && make install
}

expat() {
    cd $srcDir
    local library='expat-2.5.0'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz 
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build

    args=(
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_LIBDIR=lib
    )
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "appleSilicon" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    fi
    if [ "$OS" == "macOSx86" ]; then 
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi


    cmake "${args[@]}" ..
    make && make install
}

udunits() {
    cd $srcDir
    local library='udunits-2.2.28'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library

    args=(
        --prefix=$installDir
    )
    LDFLAGS=-L$installDir/lib/ \
    CPPFLAGS=-I$installDir/include/ \
    CC=$CC CXX=$CXX \
    ./configure "${args[@]}"
    make && make install
}

freetype() {
    cd $srcDir
    local library='freetype-2.13.0'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz
    cd $srcDir/$library

    args=(
        --prefix=$installDir
        --with-brotli=no
        --with-bzip2=no
    )

    configure "${args[@]}"
    make && make install
}

#CC=clang CXX=clang++ ./configure --prefix=/usr/local/VAPOR-Deps/2019-Aug
jpeg() {
    cd $srcDir
    rm -rf $srcDir/jpeg-9e || true
    tar xvf $srcDir/jpegsrc.v9e.tar.gz
    cd $srcDir/jpeg-9e

    echo target $MACOSX_DEPLOYMENT_TARGET

    args=(
        --prefix=$installDir
    )
    if [ "$OS" == "macOSx86" ]; then
        args+=(--host=x86_64-apple-darwin)
    fi
    CC=$CC CXX=$CXX \
    ./configure "${args[@]}"
    make && make install
}

tiff() {
    cd $srcDir
    local library='libtiff-v4.5.0'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    cd $srcDir/$library
    if [ "$OS" == "macOSx86" ] || [ "$OS" == "appleSilicon" ]; then
        glibtoolize --force
    else
        libtoolize --force
    fi
    aclocal
    autoheader
    automake --force-missing --add-missing
    autoconf
    args=(
        --prefix=$installDir
        --disable-dap
    )
    LDFLAGS=-L$installDir/lib \
    CPPFLAGS=-I$installDir/include \
    CC=$CC CXX=$CXX \
    ./configure "${args[@]}"
    make && make install
}

sqlite() {
    cd $srcDir
    local library='sqlite-autoconf-3410000'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    cd $srcDir/$library
    
    args=(
        --prefix=$installDir
    )
    CC=$CC CXX=$CXX \
    ./configure "${args[@]}"
    make && make install
}

curl() {
    cd $srcDir
    local library='curl-8.2.1' # works
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    
    args=(
        -DCMAKE_PREFIX_PATH=$installDir
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DCMAKE_LIBRARY_PATH=$installDir/lib
        -DCMAKE_INCLUDE_PATH=$installDir/include
        -DCMAKE_INSTALL_RPATH=$installDir/lib
    )

    if [ "$OS" == "macOSx86" ]; then
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi


    cmake "${args[@]}" ..

    make && make install
}

ssh() {
    cd $srcDir
    local library='libssh-0.11.0'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    
    args=(
        -DCMAKE_PREFIX_PATH=$installDir
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DCMAKE_LIBRARY_PATH=$installDir/lib
        -DCMAKE_INCLUDE_PATH=$installDir/include
    )

    if [ "$OS" == "macOSx86" ]; then
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi

    cmake "${args[@]}" ..

    make && make install
}

proj() {
    cd $srcDir
    #local library='proj-9.1.0' # does not work
    #local library='proj-6.3.1' # works
    local library='proj-7.2.1' # works
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    tar xvf proj-datumgrid-1.8.tar.gz -C $library/data
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    
    if [ "$OS" == "macOSx86" ] || [ "$OS" == "appleSilicon" ]; then
        local sqliteLib="-DSQLITE3_LIBRARY=$installDir/lib/libsqlite3.dylib"
    else
        local sqliteLib="-DSQLITE3_LIBRARY=$installDir/lib/libsqlite3.so"
    fi
    if [ "$OS" == "macOSx86" ]; then 
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
    fi


    args=(
        -DCMAKE_PREFIX_PATH=$installDir
        -DEXE_SQLITE3=$installDir/bin/sqlite3
        -DSQLITE3_INCLUDE_DIR=$installDir/include
        $sqliteLib \
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DPROJ_COMPILER_NAME=$CXX
        -DCMAKE_LIBRARY_PATH=$installDir/lib
        -DCMAKE_INCLUDE_PATH=$installDir/include
    )
    if [ "$OS" == "appleSilicon" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    elif [ "$OS" == "macOSx86" ]; then
        args+=(-DCMAKE_OSX_ARCHITECTURES=x86_64)
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    fi
    cmake "${args[@]}" ..

    make && make install
}

geotiff() {
    cd $srcDir
    local library='libgeotiff-1.7.1'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library

    echo $CC
    echo $CXX
    echo $CFLAGS
    echo $LDFLAGS
    echo $SDKROOT

  #LDFLAGS     linker flags, e.g. -L<lib dir> if you have libraries in a
  #            nonstandard directory <lib dir>
  #LIBS        libraries to pass to the linker, e.g. -l<library>
  #CPPFLAGS    (Objective) C/C++ preprocessor flags, e.g. -I<include dir> if
  #            you have headers in a nonstandard directory <include dir>
    export LDFLAGS=-L$installDir/lib
    export LIBS=-lz
    export CPPFLAGS=-I$installDir/include

    args=(
        --prefix=$installDir
        --with-zlib=$installDir
        --with-jpeg=$installDir
        --with-proj=$installDir
        --with-libtiff=$installDir
    )

    configure "${args[@]}"

    make && make install
}

xinerama() {
    cd $srcDir
    local library='xcb-proto-1.15.2'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library
    ./configure --prefix=$installDir
    make && make install

    cd $srcDir
    library='libxcb-1.15'
    export PYTHONPATH=$installDir/local/lib/python3.10/dist-packages
    tar xvf $srcDir/$library.tar.xz && cd $srcDir/$library
    PYTHON=python3 PKG_CONFIG_PATH=$installDir/share/pkgconfig ./configure --without-doxygen --docdir='${datadir}'/doc/libxcb-1.15 --prefix=$installDir
    make && make install
}

openssl() {
    cd $srcDir
    local library='openssl-1.1.1w'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library

    echo $CFLAGS
    echo $CXXFLAGS
    echo $LDFLAGS
    echo $CPPFLAGS

    args=(
        --prefix=$installDir
        --openssldir=$installDir
    )
    if [ "$OS" = "macOSx86" ]; then
        args+=(darwin64-x86_64-cc)
    elif [ "$OS" = "appleSilicon" ]; then
        args+=(darwin64-arm64-cc)
    else
        args+=(linux-x86_64)
        #args+=(linux-aarch64)
    fi
    ./Configure shared "${args[@]}"
    make && make install
}

pythonVapor() {
    cd $srcDir
    local library='cpython-3.9.16'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library

    args=(
        --prefix=$installDir
        --enable-shared
        --with-ensurepip=install
        --with-suffix=.vapor
        --enable-optimizations
    )

    pyInstallDir=$installDir
    if [ "$OS" == "macOSx86" ] || [ "$OS" == "appleSilicon" ]; then
        export PKG_CONFIG_PATH="$(brew --prefix tcl-tk)/lib/pkgconfig"
        args+=(--prefix=/usr/local/VAPOR-Deps/current/Resources)
        args+=(--with-openssl=$installDir)
        args+=(--with-tcltk-libs="$(pkg-config --libs tcl tk)")
        args+=(--with-tcltk-includes="$(pkg-config --cflags tcl tk)")
        export LDFLAGS="$LDFLAGS -L$installDir/lib"
        export CPPFLAGS="$CPPFLAGS -I$installDir/include"
        export LLVM_PROFDATA="/opt/local/bin/llvm-profdata-mp-17"
        export LD_LIBRARY_PATH="$installDir/Resources"

    	if [ "$OS" == "macOSx86" ]; then
            export CFLAGS="$CFLAGS -I/usr/local/include"
            export LDFLAGS="$LDFLAGS -L/usr/local/lib"
            args+=(--build=x86_64-apple-darwin)
        fi
        configure "${args[@]}"
        pyInstallDir=$pyInstallDir/Resources
    else
        args+=(--with-openssl=$installDir)
        CC=$CC \
        CXX=$CXX \
        CPPFLAGS=-I$installDir/include \
        LDFLAGS="-L$installDir/lib -Wl,-rpath=$installDir/lib" \
        ./configure "${args[@]}"
    fi

    make && make install

    $pyInstallDir/bin/python3.9.vapor -m pip install --upgrade pip

    # As of 5/27/2023, numpy's current version (1.24.3) fails to initialize PyEngine's call to import_array1(-1)
    $pyInstallDir/bin/python3.9.vapor -m pip install numpy==1.21.4 scipy matplotlib
}

ospray() {
    cd $srcDir
    if [ "$OS" == "appleSilicon" ]; then
        cd $srcDir/ospray/osprayM1
    elif [ "$OS" == "macOSx86" ]; then
        local library='ospray-2.11.0.x86_64.macosx'
        rm -rf ospray/$library || true
        unzip -o $srcDir/ospray/$library && cd $srcDir/$library
    else
        local library='ospray-2.11.0.x86_64.linux'
        rm -rf ospray/$library || true
        tar xvf $srcDir/ospray/$library.tar.gz && cd $srcDir/$library
    fi
    mkdir -p $installDir/Ospray
    cp -rfP * $installDir/Ospray
}

glm() {
    cd $srcDir
    local library='glm-0.9.9.8'
    rm -rf glm || true
    unzip $srcDir/$library.zip 
    cp -r $srcDir/glm/glm $installDir/include
}

gte() {
    cd $srcDir
    tar xvf GTE.tar.xz
    rsync -a GTE $installDir/include
}

images() {
    cd $srcDir
    tar xvf images.tar.xz
    rsync -a images $installDir/share
}

qt() {
    cd $srcDir
    rm -rf qt-everywhere-src-5.15.8 || true
    tar xf $srcDir/qt-everywhere-opensource-src-5.15.8.tar.xz
    mkdir -p $srcDir/qt-everywhere-src-5.15.8/build
    cd $srcDir/qt-everywhere-src-5.15.8/build
 
    if [ "$OS" == "macOSx86" ] || [ "$OS" == "appleSilicon" ]; then
        brew install gettext
        brew link gettext --force
    fi

    args=(
        -v
        -prefix $installDir
        -opensource
        -confirm-license
        -release
        -nomake examples
        -nomake tests
    )
    if [ "$OS" == "Ubuntu" ]; then
        args+=(-feature-freetype)
        args+=(-qt-freetype)
        args+=(-opengl desktop)
        # opensuse new args
        args+=(-xcb)
        args+=(-xcb-xlib)
        args+=(-bundled-xcb-xinput)
    fi

    CPPFLAGS="$CPPFLAGS -I$installDir/include" \
    LDFLAGS="$LDFLAGS -L$installDir/lib -Wl,-rpath=$installDir/lib" \
    ../configure \
    "${args[@]}" > qtConfig.txt

    make > qtMake.txt
    make install > qtInstall.txt
}

add_rpath() {
    for lib in $installDir/lib/*.dylib $installDir/Resources/lib/*.dylib; do
        fileName="$(basename $lib)"
        echo install_name_tool -id @rpath/$fileName $lib
        install_name_tool -id @rpath/$fileName $lib

        # Get the list of dependencies
        dependencies=$(otool -L "$lib" | awk '{print $1}' | grep -v ":")

        # Iterate over each dependency and replace the path with rpath
        for dep in $dependencies; do
            depName=$(basename "$dep")
            newPath="@rpath/$depName"
            if [ "$(dirname "$dep")" == "$installDir/lib" ]; then
                echo install_name_tool -change "$dep" "$newPath" "$installDir/lib/$lib"
                install_name_tool -change "$dep" "$newPath" "$lib"
            fi
            if [ "$(dirname "$dep")" == "$installDir/Resources/lib" ]; then
                echo install_name_tool -change "$dep" "$newPath" "$installDir/Resources/$lib"
                install_name_tool -change "$dep" "$newPath" "$installDir/Resources/$lib"
            fi
        done
        codesign --force -s - $lib
    done
}

renameAndCompress() {
    pwd
    cd $baseDir
    mv $installDir $archiveName
    tar cfJ $archiveName.tar.xz $archiveName
}

if [ "$OS" == "macOSx86" ]; then
    macOSx86Prerequisites
elif [ "$OS" == "appleSilicon" ]; then
    macOSPrerequisites
elif [ "$OS" == "Ubuntu" ]; then
    ubuntuPrerequisites
elif [ "$OS" == "Windows" ]; then
    windowsPrerequisites
fi

openssl
zlib
libpng
jpeg
tiff
sqlite
ssh

### m1 needs curl for proj?
#curl
###

proj
geotiff
assimp
szip
hdf5

#zstd
netcdf
expat
udunits
freetype
if [ "$OS" == "Ubuntu" ] ; then
   xinerama
fi         
ospray
glm
gte
images
pythonVapor
qt
if [ "$OS" == "macOSx86" ] || [ "$OS" == "appleSilicon" ]; then
    add_rpath
fi         
renameAndCompress
