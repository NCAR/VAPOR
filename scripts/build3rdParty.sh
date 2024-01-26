#!/bin/bash

# To install:           Uncomment 'prerequisites' and the library name you'd like, 
#                       listed at the bottom of this file.  Then run build.sh.
#
# To uninstall:         Re-install your target library, then run the following
#                       command from the build directory.
#
#                       xargs rm < install_manifest.txt

set -e

while getopts o:b:i flag
do
    case "${flag}" in
        b) srcDir=${OPTARG};;
        i) installDir=${OPTARG};;
        o) OS=${OPTARG};;
    esac
done

#OS="CentOS"
baseDir='/usr/local/VAPOR-Deps'
if [ "$OS" != "suse" ]; then
    srcDir="$baseDir/2023-Jun-src"
    archiveName="2023-Jun"
else
    srcDir="$baseDir/2023-Sept-src"
    archiveName="2023-Sept"
fi
echo srcDir $srcDir
echo OS ${OS}

installDir="$baseDir/current"
getMacOSMinVersion() {
    if [ "$OS" == "macOSx86" ]; then
        echo "10.15.0"
    elif [ "$OS" = "M1" ]; then
        echo "12.0.0"
    fi
}
macOSMinVersion=$(getMacOSMinVersion)

shopt -s expand_aliases
alias make='make -j8'
alias

if [ "$OS" == "CentOS" ]; then
    shopt -s expand_aliases
    alias cmake=cmake3
    alias
fi

macOSx86Prerequisites() {
    brew uninstall python@3.9
    macOSPrerequisites
}

macOSPrerequisites() {
    export MACOSX_DEPLOYMENT_TARGET=$macOSMinVersion
    CC='clang'
    CXX='clang++'
    brew install cmake
    brew install autoconf
    brew install atool
    brew install libtool
    brew install automake
    #brew install xz zlib openssl
    brew install xz
    #brew install python@3.9
    #brew install pkg-config openssl@1.1 xz gdbm tcl-tk # https://devguide.python.org/getting-started/setup-building/index.html#macos-and-os-x
    brew install pkg-config xz gdbm tcl-tk # https://devguide.python.org/getting-started/setup-building/index.html#macos-and-os-x
    #brew install gettext
    #brew link gettext --force
    brew uninstall --ignore-dependencies gettext
}

susePrerequisites() {
    CC='gcc'
    CXX='g++'
    zypper update -y

    zypper install -y cmake

    zypper install -y \
        libqt5-qtbase-devel \
        libicu-devel \
        m4 \
        libXau-devel \
        autoconf \
        libtool \
        libxcb-xinerama0 \
        pkg-config \
        unzip \
        gcc7-fortran
    
    # Qt
    zypper install -y \
        libX11-xcb1 \
        libXinerama-devel \
        freeglut-devel \
        libGLU1 \
        libXrender-devel \
        libXi-devel \
        libxkbcommon-devel \
        libxkbcommon-x11-devel \
        libSM-devel \
        libICE-devel

    # Additional suse dependencies https://github.com/bincrafters/community/issues/1229
    zypper install -y \
        xcb-util-wm-devel \
        xcb-util-image-devel \
        xcb-util-keysyms-devel \
        libxcb-devel \
        xcb-util-renderutil-devel
}

ubuntuPrerequisites() {
    CC='gcc'
    CXX='g++'
    apt update -y
    apt upgrade -y

    # all for cmake
    apt-get update
    apt-get install -y gpg wget
    wget http://archive.ubuntu.com/ubuntu/pool/main/o/openssl/libssl1.1_1.1.1f-1ubuntu2_amd64.deb
    dpkg -i libssl1.1_1.1.1f-1ubuntu2_amd64.deb
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null
    DEBIAN_FRONTEND=noninteractive apt install -y software-properties-common
    apt-add-repository -y 'deb https://apt.kitware.com/ubuntu/ focal main'
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

fedoraPrerequisites() {
    yum install -y \
    libxcb-devel.x86_64 \
    libX11-xcb.x86_64 \
    libxcb.x86_64 \
    freeglut-devel.x86_64 \
    mesa-libGLU-devel.x86_64 \
    libXrender-devel.x86_64 \
    libXi-devel.x86_64 \
    libxkbcommon-devel.x86_64 \
    libxkbcommon-x11-devel.x86_64 \
    
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
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "M1" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    fi

    cmake "${args[@]}" ..
    make && make install
}

assimp() {
    cd $srcDir
    if [ "$OS" == "CentOS" ]; then
        local library='assimp-5.1.6'
    else
        local library='assimp-5.2.5' #requires c++17 compiler
    fi
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
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "M1" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
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
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "M1" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
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
    elif [ "$OS" == "M1" ]; then
        tar xvf hdf5/hdf5-$hdfVersion-Std-macos11m1_64-clang.tar.gz && cd hdf
        ./HDF5-$hdfVersion-Darwin.sh --prefix=$installDir --exclude-subdir --skip-license
    else
        tar xvf hdf5/hdf5-$hdfVersion-Std-centos7_64-7.2.0.tar.gz && cd hdf
        ./HDF5-$hdfVersion-Linux.sh --prefix=$installDir --exclude-subdir --skip-license
    fi

    ln -fs $installDir/HDF_Group/HDF5/$hdfVersion/lib/plugin/ $installDir/share/plugins
}

netcdf() {
    cd $srcDir
    local library='netcdf-c-4.9.1'
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build

    args=(
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DCMAKE_PREFIX_PATH="$installDir/HDF_Group/HDF5/$hdfVersion"
        -DCMAKE_INSTALL_LIBDIR=lib
        -DENABLE_BYTERANGE=False
        -DENABLE_DAP=False
        -DCMAKE_BUILD_TYPE=Release
    )
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "M1" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    fi

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
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "M1" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
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
    )
    CC=$CC CXX=$CXX \
    ./configure "${args[@]}"
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
    if [ "$OS" == "macOSx86" ] || [ "$OS" = "M1" ]; then
        args+=(--host=arm-apple-darwin)
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
    if [ "$OS" == "macOSx86" ] || [ "$OS" == "M1" ]; then
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
    echo $LD_LIBRARY_PATH
    echo $DYLD_LIBRARY_PATH
    cd $srcDir
    local library='curl-8.2.1' # works
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    
    args=(
        -DCMAKE_PREFIX_PATH=$installDir
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DPROJ_COMPILER_NAME=$CXX
        -DCMAKE_LIBRARY_PATH=$installDir/lib
        -DCMAKE_INCLUDE_PATH=$installDir/include
        -DCMAKE_INSTALL_RPATH=$installDir/lib
    )

    cmake "${args[@]}" ..

    make && make install
}

ssh() {
    cd $srcDir
    local library='libssh-0.10.0' # works
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.xz
    mkdir -p $srcDir/$library/build && cd $srcDir/$library/build
    
    args=(
        -DCMAKE_PREFIX_PATH=$installDir
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_INSTALL_PREFIX=$installDir
        -DPROJ_COMPILER_NAME=$CXX
        -DCMAKE_LIBRARY_PATH=$installDir/lib
        -DCMAKE_INCLUDE_PATH=$installDir/include
    )

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
    
    if [ "$OS" == "macOSx86" ] || [ "$OS" == "M1" ]; then
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
        -DCMAKE_LIBRARY_PATH=$installDir/lib
        -DCMAKE_INCLUDE_PATH=$installDir/include
    )
    if [ "$OS" == "M1" ]; then
        args+=(-DCMAKE_OSX_ARCHITECTURES=arm64)
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    elif [ "$OS" == "macOSx86" ]; then
        args+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=$macOSMinVersion)
    elif [ "$OS" == "suse" ]; then
        args+=(-DCURL_INCLUDE_DIR=$installDir/include)
        args+=(-DCURL_LIBRARY=$installDir/lib/libcurl.so)
    fi
    cmake "${args[@]}" ..

    make && make install
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

    #if [ "$OS" == "suse" ]; then
    #    LDFLAGS=-L/usr/lib64 -L$installDir/lib \
    #else
    #    LDFLAGS=-L$installDir/lib \
    #fi
    CPPFLAGS=-I$installDir/include \
    CC=$CC \
    CXX=$CXX \
    ./configure "${args[@]}"

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
    if [ "$OS" != "suse" ]; then
        local library='openssl-1.1.1t'
    else
        local library='openssl-1.1.1w'
    fi
    rm -rf $library || true
    tar xvf $srcDir/$library.tar.gz && cd $srcDir/$library

    args=(
        --prefix=$installDir
        --openssldir=$installDir
    )

    ./config shared "${args[@]}"
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
    )
    if [ "$OS" != "CentOS" ]; then
        args+=(--enable-optimizations)
    fi

    #LDFLAGS="-L$installDir/lib -L$(brew --prefix zlib)/lib -I$(brew --prefix openssl)/lib" \
    #CPPFLAGS="-I$installDir/include -I$(brew --prefix zlib)/include -I$(brew --prefix openssl)/include" \
    if [ "$OS" = "macOSx86" ] || [ "$OS" = "M1" ]; then
        args+=(--with-openssl=$(brew --prefix openssl@1.1))
        args+=(--with-tcltk-libs="$(pkg-config --libs tcl tk)")
        args+=(--with-tcltk-includes="$(pkg-config --cflags tcl tk)")
        #args+=(--with-macosx-version-min=$macOSMinVersion)
        export PKG_CONFIG_PATH="$(brew --prefix tcl-tk)/lib/pkgconfig"; \
        CC=$CC \
        CXX=$CXX \
        LDFLAGS="-L$installDir/lib" \
        CPPFLAGS="-I$installDir/include" \
        ./configure "${args[@]}"
    else
        args+=(--with-openssl=$installDir)
        CC=$CC \
        CXX=$CXX \
        CPPFLAGS=-I$installDir/include \
        LDFLAGS="-L$installDir/lib -Wl,-rpath=$installDir/lib" \
        ./configure "${args[@]}"
    fi

    make && make install

    $installDir/bin/python3.9.vapor -m pip install --upgrade pip

    # As of 5/27/2023, numpy's current version (1.24.3) fails to initialize PyEngine's call to import_array1(-1)
    $installDir/bin/python3.9.vapor -m pip install numpy==1.21.4 scipy matplotlib
}

ospray() {
    cd $srcDir
    if [ "$OS" == "M1" ]; then
        # 6/2/2023 - CircleCI was having difficulty targeting arm64 for Ospray's build.
        #            Workaround: compile Ospray locally and add it to the source .tar.xz bundle
        cd $srcDir/ospray/osprayM1
    elif [ "$OS" == "macOSx86" ]; then
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
        rm -rf qt-everywhere-src-5.15.8 || true
        tar xf $srcDir/qt-everywhere-opensource-src-5.15.8.tar.xz
        mkdir -p $srcDir/qt-everywhere-src-5.15.8/build
        cd $srcDir/qt-everywhere-src-5.15.8/build

        if [ "$OS" == "macOSx86" ] || [ "$OS" == "M1" ]; then
            brew install gettext
            brew link gettext --force
        fi
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
    if [ "$OS" == "Ubuntu" ] || [ "$OS" = "suse" ]; then
        args+=(-feature-freetype)
        args+=(-qt-freetype)
        args+=(-opengl desktop)
        # opensuse new args
        args+=(-xcb)
        args+=(-xcb-xlib)
        args+=(-bundled-xcb-xinput)
    fi

    CPPFLAGS=-I$installDir/include \
    LDFLAGS="-L$installDir/lib -Wl,-rpath=$installDir/lib" \
    CC=$CC \
    CXX=$CXX \
    ../configure \
    "${args[@]}" > qtConfig.txt

    make > qtMake.txt
    make install > qtInstall.txt
}

add_rpath() {
    for lib in $installDir/lib/*.dylib; do
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
        done
        codesign --force -s - $lib
    done
}

#baseDir='/usr/local/VAPOR-Deps'
#srcDir="$baseDir/2023-Jun-src"
#installDir="$baseDir/current"
#archiveName="2023-Jun"
renameAndCompress() {
    pwd
    cd $baseDir
    mv $installDir $archiveName
    tar cfJ $archiveName-$OS.tar.xz $archiveName
}

if [ "$OS" == "macOSx86" ]; then
    macOSx86Prerequisites
elif [ "$OS" == "M1" ]; then
    macOSPrerequisites
elif [ "$OS" == "Ubuntu" ]; then
    ubuntuPrerequisites
elif [ "$OS" == "CentOS" ]; then
    centosPrerequisites
elif [ "$OS" == "Windows" ]; then
    windowsPrerequisites
elif [ "$OS" == "suse" ]; then
    susePrerequisites
fi

openssl
libpng
jpeg
tiff
sqlite
ssh
curl
proj
geotiff

#    openssl
#fi
zlib
pythonVapor
assimp
szip
hdf5
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
qt
if [ "$OS" == "macOSx86" ] || [ "$OS" == "M1" ]; then
    add_rpath
fi         
renameAndCompress
