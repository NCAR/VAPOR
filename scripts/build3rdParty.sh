#!/bin/bash

# To install:           Uncomment 'prerequisites' and the library name you'd like, 
#                       listed at the bottom of this file.  Then run build.sh.
#
# To uninstall:         Re-install your target library, then run the following
#                       command from the build directory.
#
#                       xargs rm < install_manifest.txt

set -e

#OS="CentOS"
baseDir='/usr/local/VAPOR-Deps'
srcDir="$baseDir/2023-Mar-src"
installDir="$baseDir/current"
archiveName="2023-Mar"

while getopts o:b:i flag
do
    case "${flag}" in
        b) srcDir=${OPTARG};;
        i) installDir=${OPTARG};;
        o) OS=${OPTARG};;
    esac
done

if [ "$OS" = "CentOS" ]; then
    shopt -s expand_aliases
    alias cmake=cmake3
    alias
fi

osxPrerequisites() {
    CC='clang'
    CXX='clang++'
    brew install cmake
    brew install autoconf
    brew install atool
    brew install libtool
    brew install automake
    brew install xz zlib openssl
    brew install python@3.9
    brew install pkg-config openssl@1.1 xz gdbm tcl-tk # https://devguide.python.org/getting-started/setup-building/index.html#macos-and-os-x
}

ubuntuPrerequisites() {
    CC='gcc'
    CXX='g++'
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

    apt install -y \
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

centosPrerequisites() {
    CC='gcc'
    CXX='g++'
	yum install -y epel-release
	yum update -y
	yum install -y \
        kernel-devel \
        gcc \
        gcc-c++ \
        cmake3 \
        make \
        xz-devel \
        zlib-devel \
        openssl-devel \
        expat-devel \
        libcurl-devel \
        which \
        mesa-libGL-devel \
        mesa-libGLU-devel
	yum update -y

    #shopt -s expand_aliases
    #alias cmake='cmake3'
    #shopt -s expand_aliases
    #echo alias cmake=\'cmake3\' >> ~/.bashrc
    #. ~/.bashrc
    #source ~/.bashrc

    cmake --version
    cmake3 --version

    yum groupinstall -y "Development Tools"

	#curl -LO https://github.com/Kitware/CMake/releases/download/v3.26.0/cmake-3.26.0-linux-x86_64.tar.gz
	#tar -xvf cmake-3.26.0.tar.gz
	#mv cmake-3.26.0 /usr/local/cmake
	#echo 'export PATH="/usr/local/cmake/bin:$PATH"' >> ~/.bashrc
	#source ~/.bashrc
}

libpng() {
    cd $srcDir
    local library='libpng-1.6.39'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz 
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    cmake \
    -DCMAKE_INSTALL_PREFIX=$installDir \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_LIBDIR=lib \
    ..
    make -j4 && make install
}

assimp() {
    cd $srcDir
    if [ "$OS" == "CentOS" ]; then
        local library='assimp-5.1.6'
    else
        local library='assimp-5.2.5' #requires c++17
    fi
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    cmake \
    -DCMAKE_INSTALL_PREFIX=$installDir \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DCMAKE_CXX_FLAGS="-O3 -Wno-error=deprecated-declarations" \
    -DASSIMP_BUILD_TESTS=OFF \
    ..
    make -j4 && make install
}

zlib() {
    cd $srcDir
    local library='zlib-1.2.13'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    cmake -DCMAKE_INSTALL_PREFIX=$installDir -DCMAKE_BUILD_TYPE=Release ..
    make -j4 && make install
}

#Note: after configuration, need to make sure both zlib and szlib are enabled!!
# How are we supposed do do that?  Configure does not indicate yes or no...
szip() {
    cd $srcDir
    local library='szip-2.1.1'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    cd $srcDir/$library
    CC=$CC CXX=$CXX ./configure --prefix=$installDir
    make -j4 && make install
}

#hdfVersion='1.14.0'
#hdfVersion='1.13.3'
hdfVersion='1.12.2'
hdf5() {
    cd $srcDir
    if [ "$OS" == "Ubuntu" ] || [ "$OS" == "CentOS" ]; then
        #tar xvf hdf5/hdf5-$hdfVersion-Std-centos7_64.tar.gz && cd hdf              # use this line for versions > 12.12.2
        tar xvf hdf5/hdf5-$hdfVersion-Std-centos7_64-7.2.0.tar.gz && cd hdf         # use this line for versions = 12.12.2
        ./HDF5-$hdfVersion-Linux.sh --prefix=$installDir --exclude-subdir --skip-license
    elif [ "$OS" == "OSX" ]; then
        tar xvf hdf5/hdf5-$hdfVersion-Std-macos11_64-clang.tar.gz && cd hdf
        ./HDF5-$hdfVersion-Darwin.sh --prefix=$installDir --exclude-subdir --skip-license
    elif [ "$OS" == "M1" ]; then
        tar xvf hdf5/hdf5-$hdfVersion-Std-macos11m1_64-clang.tar.gz && cd hdf
        ./HDF5-$hdfVersion-Darwin.sh --prefix=$installDir --exclude-subdir --skip-license
    fi

    ln -s $installDir/HDF_Group/HDF5/$hdfVersion/lib/plugin/ $installDir/share/plugins
}

netcdf() {
    cd $srcDir
    local library='netcdf-c-4.9.1'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build

    cmake \
    -DCMAKE_INSTALL_PREFIX=$installDir \
    -DCMAKE_PREFIX_PATH="$installDir/HDF_Group/HDF5/$hdfVersion" \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DENABLE_BYTERANGE=False \
    -DENABLE_DAP=False \
    -DCMAKE_BUILD_TYPE=Release ..

    make && make install
}

expat() {
    cd $srcDir
    local library='expat-2.5.0'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz 
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    cmake \
    -DCMAKE_INSTALL_PREFIX=$installDir \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_LIBDIR=lib \
    ..
    make -j4 && make install
}

udunits() {
    cd $srcDir
    local library='udunits-2.2.28'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library
    LDFLAGS=-L$installDir/lib/ \
    CPPFLAGS=-I$installDir/include/ \
    CC=$CC CXX=$CXX \
    ./configure \
    --prefix=$installDir
    make -j4 && make install
}

freetype() {
    cd $srcDir
    local library='freetype-2.13.0'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz
    cd $srcDir/$library
    CC=$CC CXX=$CXX ./configure --prefix=$installDir
    make -j4 && make install
}

#CC=clang CXX=clang++ ./configure --prefix=/usr/local/VAPOR-Deps/2019-Aug
jpeg() {
    cd $srcDir
    rm -rf $srcDir/jpeg-9e || true
    tar xvf $srcDir/jpegsrc.v9e.tar.gz
    cd $srcDir/jpeg-9e
    CC=$CC CXX=$CXX ./configure --prefix=$installDir
    make -j4 && make install
}

tiff() {
    cd $srcDir
    local library='libtiff-v4.5.0'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    cd $srcDir/$library/build

    cmake \
    -DCMAKE_INSTALL_PREFIX=$installDir \
    -DCMAKE_INSTALL_LIBDIR=lib \
    .. 
    make -j4 && make install
}

oldTiff() {
    cd $srcDir
    local library='libtiff-v4.5.0'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    cd $srcDir/$library
    if [ "$OS" == "OSX" ] || [ "$OS" == "M1" ]; then
        glibtoolize --force
    else
        libtoolize --force
    fi
    aclocal
    autoheader
    automake --force-missing --add-missing
    autoconf
    LDFLAGS=-L$installDir/lib \
    CPPFLAGS=-I$installDir/include \
    CC=$CC CXX=$CXX \
    ./configure \
    --prefix=$installDir \
    --disable-dap
    make -j4 && make install
}

sqlite() {
    cd $srcDir
    local library='sqlite-autoconf-3410000'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    cd $srcDir/$library
    CC=$CC CXX=$CXX ./configure --prefix=$installDir
    make -j4 && make install
}

proj() {
    cd $srcDir
    #local library='proj-9.1.0' # does not work
    #local library='proj-6.3.1' # works
    local library='proj-7.2.1' # ?
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    tar xvf proj-datumgrid-1.8.tar.gz -C $library/data
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    
    if [ "$OS" == "OSX" ] || [ "$OS" == "M1" ]; then
        local sqliteLib="-DSQLITE3_LIBRARY=$installDir/lib/libsqlite3.dylib"
    else
        local sqliteLib="-DSQLITE3_LIBRARY=$installDir/lib/libsqlite3.so"
    fi

    args=(
        -DCMAKE_PREFIX_PATH=$installDir
        -DEXE_SQLITE3=$installDir/bin/sqlite3
        -DSQLITE3_INCLUDE_DIR=$installDir/include
        $sqliteLib \
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DPROJ_COMPILER_NAME=$CXX
    )
    if [ "$OS" == "M1" ]; then
        args+=(-DCMAKE_OSX_ARCHITECTURES=arm64)
    fi
    cmake "${args[@]}" ..

    #cmake \
    #-DCMAKE_PREFIX_PATH=$installDir \
    #-DEXE_SQLITE3=$installDir/bin/sqlite3 \
    #-DSQLITE3_INCLUDE_DIR=$installDir/include \
    #$sqliteLib \
    #-DCMAKE_INSTALL_LIBDIR=lib \
    #-DCMAKE_INSTALL_PREFIX=$installDir \
    #-DCMAKE_OSX_ARCHITECTURES=arm64 \
    #-DPROJ_COMPILER_NAME=$CXX \
    #..

    make -j4 && make install
}

#CPPFLAGS=-I$/usr/local/VAPOR-Deps/current/include LDFLAGS=-L/usr/local/VAPOR-Deps/current/lib CC=gcc CXX=g++ ./configure -prefix=/usr/local/VAPOR-Deps/current --with-zlib=yes --with-jpeg=yes --with-proj=/usr/local/VAPOR-Deps/current --with-libtiff=/usr/local/VAPOR-Deps/current/lib64
geotiff2() {
    cd $srcDir
    local library='libgeotiff-1.7.1'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && mkdir $srcDir/$library/build && cd $_

    args=(
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DPROJ_DIR=$installDir
#        -DTIFF_DIR=$installDir
#        -DZLIB_DIR=$installDir
        -DWITH_JPEG=ON
        -DWITH_ZLIB=ON
    )
    if [ "$OS" == "M1" ]; then
        args+=(-DCMAKE_OSX_ARCHITECTURES=arm64)
    fi
    if [ "$OS" == "M1" ] || [ "$OS" == "OSX" ]; then
        args+=(-DCMAKE_SKIP_RPATH=ON)
        args+=(-DCMAKE_SKIP_INSTALL_RPATH=ON)
    fi
    #CPPFLAGS=-I$installDir/include \
    #LDFLAGS="-L$installDir/lib -Wl" \
    #CC=$CC \
    #CXX=$CXX \
    cmake "${args[@]}" ..

    make -j4 && make install
}

geotiff() {
    cd $srcDir
    local library='libgeotiff-1.7.1'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library

    args=(
        --prefix=$installDir
        --with-zlib=yes
        --with-jpeg=yes
        --with-proj=$installDir
        --with-libtiff=$installDir
    )
    #if [ "$OS" == "M1" ]; then
    #    args+=(--build=arm64)
    #fi

    #echo "${args[@]}"

    CPPFLAGS=-I$installDir/include \
    LDFLAGS=-L$installDir/lib \
    CC=$CC \
    CXX=$CXX \
    ./configure "${args[@]}"


    # the old way.  need to specify arm64.
    #LDFLAGS=-L$installDir/lib/ \
    #CPPFLAGS=-I$installDir/include/ \
    #CC=$CC CXX=$CXX \
    #./configure \
    #--prefix=$installDir \
    #--with-zlib=yes \
    #--with-jpeg=yes \
    #--with-proj=$installDir

    make -j4 && make install
}

xinerama() {
    cd $srcDir
    local library='xcb-proto-1.15.2'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library
    ./configure --prefix=$installDir
    make -j4 && make install

    cd $srcDir
    library='libxcb-1.15'
    export PYTHONPATH=$installDir/local/lib/python3.10/dist-packages
    tar xvf $srcDir/$library.tar.xz && cd $srcDir/$library
    PYTHON=python3 PKG_CONFIG_PATH=$installDir/share/pkgconfig ./configure --without-doxygen --docdir='${datadir}'/doc/libxcb-1.15 --prefix=$installDir
    make -j4 && make install
}

openssl() {
    cd $srcDir
    local library='openssl-1.1.1t'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library
    ./config shared --prefix=$installDir --openssldir=$installDir
    make -j4 && make install
}

pythonVapor() {
    cd $srcDir
    local library='cpython-3.9.16'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library
    if [ "$OS" = "OSX" ] || [ "$OS" = "M1" ]; then
        export PKG_CONFIG_PATH="$(brew --prefix tcl-tk)/lib/pkgconfig"; \
        CC=$CC \
        CXX=$CXX \
        LDFLAGS="-L$installDir/lib -L$(brew --prefix zlib)/include -I$(brew --prefix openssl)/include" \
        CPPFLAGS="-I$installDir/include -I$(brew --prefix zlib)/include -I$(brew --prefix openssl)/include" \
        ./configure \
        --prefix=$installDir \
        --enable-shared \
        --with-ensurepip=install \
        --with-suffix=.vapor \
        --enable-optimizations \
        --with-openssl=$(brew --prefix openssl@1.1) \
        --with-tcltk-libs="$(pkg-config --libs tcl tk)" \
        --with-tcltk-includes="$(pkg-config --cflags tcl tk)"
    elif [ "$OS" = "CentOS" ]; then
        CPPFLAGS=-I$installDir/include \
        LDFLAGS="-L$installDir/lib -Wl,-rpath=$installDir/lib" \
        CC=$CC \
        CXX=$CXX \
        ./configure \
        --prefix=$installDir \
        --enable-shared \
        --with-ensurepip=install \
        --with-suffix=.vapor \
        --with-openssl=$installDir
    else
        CPPFLAGS=-I$installDir/include \
        LDFLAGS="-L$installDir/lib -Wl,-rpath=$installDir/lib" \
        CC=$CC \
        CXX=$CXX \
        ./configure \
        --prefix=$installDir \
        --enable-shared \
        --with-ensurepip=install \
        --with-suffix=.vapor \
        --enable-optimizations \
        --with-openssl=$installDir
    fi

    make -j4 && make install

    $installDir/bin/python3.9.vapor -m pip install --upgrade pip
    $installDir/bin/pip3 install --upgrade --target $installDir/lib/python3.9/site-packages numpy scipy matplotlib
}

ospray() {
    cd $srcDir
    if [ "$OS" == "M1" ]; then
        cd $srcDir/ospray/osprayM1
        # The following source build results with the following error on CircleCI:
        #   Error executing ISPC executable
        #   Bad CPU type in executable
        #
        #git clone https://github.com/ospray/ospray.git ospraySrc
        #cd ospraySrc && git checkout v2.11.0
        #mkdir build && cd build
        #cmake \
        #    -DBUILD_JOBS=8 \
        #    -DBUILD_TBB_FROM_SOURCE=ON \
        #    $srcDir/ospraySrc/scripts/superbuild/
        #cmake --build .
        #mv install/ospray/* $installDir/Ospray
    elif [ "$OS" == "OSX" ]; then
        local library='ospray-2.11.0.x86_64.macosx'
        rm -rf ospray/$library || true
        unzip $srcDir/ospray/$library && cd $srcDir/$library
    else
        local library='ospray-2.11.0.x86_64.linux'
        rm -rf ospray/$library || true
        tar xvf $srcDir/ospray/$library.tar.gz && cd $srcDir/$library
    fi
    mkdir -p $installDir/Ospray
    cp -rP * $installDir/Ospray
}

glm() {
    cd $srcDir
    local library='glm-0.9.9.8'
    unzip $srcDir/$library.zip && cp -r $srcDir/glm/glm $installDir/include
    #unzip $library.zip && mkdir -p glm/build
    #cd glm/build
    #cmake -DCMAKE_INSTALL_PREFIX=$installDir -DCMAKE_BUILD_TYPE=Release ..
    #make -j4 && make install
}

gte() {
    cd $srcDir
    tar xvf GTE.tar.xz
    mv GTE $installDir/include
}

images() {
    cd $srcDir
    tar xvf images.tar.xz
    mv images $installDir/share
}

qt() {
    cd $srcDir
    if [ "$OS" == "CentOS" ]; then
        local library='qt-everywhere-src-5.13.2'
        tar xf $srcDir/$library.tar.xz
        mkdir -p $srcDir/$library/build
        cd $srcDir/$library/build
    else 
        tar xf $srcDir/qt-everywhere-opensource-src-5.15.8.tar.xz
        mkdir -p $srcDir/qt-everywhere-src-5.15.8/build
        cd $srcDir/qt-everywhere-src-5.15.8/build
    fi

    #mkdir -p $srcDir/$library/build
    #cd $srcDir/$library/build

    args=(
        -v
        -prefix $installDir
        -opensource
        -confirm-license
        -release
        -nomake examples
        -nomake tests
    )
    if [ "$OS" == "Ubuntu" ] || [ "$OS" = "CentOS" ]; then
        args+=(-feature-freetype)
        #args+=(-fontconfig)
        args+=(-qt-freetype)
    fi
    #args+=(> qtConfig.txt)

    #echo "${args[@]}"

    CPPFLAGS=-I$installDir/include \
    LDFLAGS="-L$installDir/lib -Wl,-rpath=$installDir/lib" \
    CC=$CC \
    CXX=$CXX \
    ../configure \
    "${args[@]}" > qtConfig.txt

    make -j4 > qtMake.txt
    make install > qtInstall.txt
}

#baseDir='/usr/local/VAPOR-Deps'
#srcDir="$baseDir/2023-Mar-src"
#installDir="$baseDir/current"
#archiveName="2023-Mar"
renameAndCompress() {
    pwd
    cd $baseDir
    mv $installDir $archiveName
    tar cfJ $archiveName-$OS.tar.xz $archiveName
    #bundle="$baseDir/$archiveName-$OS"
    #tar cfJ $bundle.tar.xz $bundle
    #mv $bundle.tar.xz /usr/local/VAPOR-Deps
}

if [ "$OS" == "OSX" ] || [ "$OS" == "M1" ]; then
    osxPrerequisites
elif [ "$OS" == "Ubuntu" ]; then
    ubuntuPrerequisites
elif [ "$OS" == "CentOS" ]; then
    centosPrerequisites
elif [ "$OS" == "Windows" ]; then
    windowsPrerequisites
fi

openssl
pythonVapor
zlib
libpng
assimp
szip
hdf5
netcdf
expat
udunits
freetype
jpeg
tiff
sqlite
proj
geotiff
#geotiff2
if [ "$OS" == "Ubuntu" ] ; then
   xinerama
fi         
ospray
glm
gte
images
qt
if [ "$OS" == "OSX" ] || [ "$OS" == "M1" ]; then
   python /Users/distiller/project/buildutils/OSX_PostBuild.py
fi         
renameAndCompress
