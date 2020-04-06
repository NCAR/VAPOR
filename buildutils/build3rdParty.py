import os
import sys
import platform
import subprocess
import argparse
import glob
import distro
from collections import OrderedDict

InstallationDir = "/usr/local/VAPOR-Deps/2019-Aug-test2"
BuildDir = r'build/'
#InstallationDir = "/home/pearse/2019-Aug-src"
CCompiler = "gcc"
CppCompiler = "g++"
CMake = "cmake"
Make = "make"
QtVersion = "5.13.2"

if ( distro.id() == 'darwin' ):
    CCompiler = "clang"
    CppCompiler = "clang++"

'''("zlib", 
"CMAKE_EXE "
"CC=C_COMPILER "
"CXX=CPP_COMPILER "
"-DCMAKE_INSTALL_PREFIX=INSTALLATION_DIR "
"-DCMAKE_BUILD_TYPE=Release "
".. "
"&& MAKE_EXE "
"&& MAKE_EXE install"),'''
Libraries = OrderedDict( [
    ("assimp", 
        "CMAKE_EXE "
        "CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "-DCMAKE_INSTALL_PREFIX=INSTALLATION_DIR "
        "-DCMAKE_BUILD_TYPE=Release "
        ".. "
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("szip", 
        "rm -rf /tmp/szlib "
        "&& git clone https://github.com/taqu/szlib.git /tmp/szlib "
        "&& mv /tmp/szlib/szlib.h INSTALLATION_DIR/include "
        "&& CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "LDFLAGS=-LINSTALLATION_DIR/lib "
        "CPPFLAGS=-IINSTALLATION_DIR/include "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("hdf5",
        "CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "LDFLAGS=-LINSTALLATION_DIR/lib "
        "CPPFLAGS=-IINSTALLATION_DIR/include "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "--with-szlib=INSTALLATION_DIR "
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("netcdf", 
        "CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "LDFLAGS=-LINSTALLATION_DIR/lib "
        "CPPFLAGS=-IINSTALLATION_DIR/include "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "--disable-dap "
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("udunits", 
        "CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("freetype",
        "CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("jpeg", 
        "CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("tiff", 
        "CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "LDFLAGS=-LINSTALLATION_DIR/lib "
        "CPPFLAGS=-IINSTALLATION_DIR/include "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "--disable-dap"
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("sqlite", 
        "CC=C_COMPILER " "CXX=CPP_COMPILER "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("proj",
        "CMAKE_EXE "
        "-DEXE_SQLITE3=INSTALLATION_DIR/bin/sqlite3 " 
        "-DSQLITE3_INCLUDE_DIR=INSTALLATION_DIR/include "
        "-DSQLITE3_LIBRARY=SQLITE_LIBRARY "
        "-DPROJ_COMPILER_NAME=CPP_COMPILER "
        "-DCMAKE_INSTALL_PREFIX=INSTALLATION_DIR "
        ".. "
        "&& unzip ../../../proj-datumgrid-1.8.zip -d data "
        "&& MAKE_EXE "
        "&& MAKE_EXE install "),
    ("libgeotiff",
        "CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "LDFLAGS=-LINSTALLATION_DIR/lib "
        "CPPFLAGS=-IINSTALLATION_DIR/include "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "--with-zlib=yes "
        "--with-jpeg=yes "
        "&& MAKE_EXE "
        "&& MAKE_EXE install"),
    ("glew",
        "sed -i 's:GLEW_DEST ?= /usr:GLEW_DEST ?= INSTALLATION_DIR:' Makefile "
        "&& sed -i 's:GLEW_PREFIX ?= /usr:GLEW_PREFIX ?= INSTALLATION_DIR:' Makefile "
        "&& sed -i 's:LIBDIR = $(GLEW_DEST)/lib64:LIBDIR = $(GLEW_DEST)/lib:' config/Makefile.linux "
        "&& sed -i 's:GLEW_DEST = /usr/local:GLEW_DEST = INSTALLATION_DIR:' config/Makefile.darwin "
        "&& make "
        "&& make install"),
    ("Python",
        "CPPFLAGS=\"-IINSTALLATION_DIR/include\" "
        "LDFLAGS=\"-LINSTALLATION_DIR/lib -Wl,-rpath,INSTALLATION_DIR/lib\" "
        "CC=C_COMPILER "
        "CXX=CPP_COMPILER "
        "./configure "
        "--prefix=INSTALLATION_DIR "
        "--enable-shared "
        "--with-ensurepip=install "
        "--with-suffix=.vapor "
        "&& make "
        "&& make install "
        "&& INSTALLATION_DIR/bin/pip3 install --upgrade pip "
        "&& INSTALLATION_DIR/bin/pip3 install "
        "--upgrade "
        "--target INSTALLATION_DIR/lib/python3.6/site-packages numpy scipy matplotlib") ,
])
Libraries = OrderedDict( [
    ("qt",
	    "rm -rf build/qt "
        "&& git clone git://code.qt.io/qt/qt5.git build/qt"
        "&& cd build/qt "
        "&& git checkout QT_VERSION "
        "&& perl init-repository "
        "&& ../qt5/configure "
        "-prefix INSTALLATION_DIR/Qt "
        "-release "
        "-confirm-license "
        "-developer-build "
        "-opensource "
        "-nomake examples "
        "-nomake tests "
        "&& make "
        "&& make install")
    ]
)

def checkDarwinPythonDeps():
    formulae = [ "xz", "zlib", "openssl" ]

    for formula in formulae:
        command = "brew ls --versions " + formula
        p = subprocess.Popen( command, stdout=subprocess.PIPE, shell=True )
        output = p.stdout.readlines()
        if ( len(output) == 0 ):
            print( "Missing package for Python: " + formula )
            print( "Please install it with homebrew." )
            print( "brew install " + formula )
            print( "Exiting." )
            exit( -1 )

def checkCentOSPythonDeps():
    formulae = [ "xz-devel", "zlib-devel", "openssl-devel" ]

    for formula in formulae:
        command = "yum info " + formula
        p = subprocess.Popen( command, stdout=subprocess.PIPE, shell=True )
        streamdata = p.communicate()[0]
        if ( p.returncode != 0 ):
            print( "Missing package for Python: " + formula )
            print( "Please install it with yum." )
            print( "yum install " + formula )
            print( "Exiting." )
            exit( -1 )

def checkUbuntuPythonDeps():
    formulae = [ "xz-utils", "zlib1g-dev"]#, "libssl-devel" ]

    for formula in formulae:
        command = "dpkg -l " + formula
        p = subprocess.Popen( command, stdout=subprocess.PIPE, shell=True )
        streamdata = p.communicate()[0]
        if ( p.returncode != 0 ):
            print( "Missing package for Python: " + formula )
            print( "Please install it with apt-get." )
            print( "apt-get install " + formula )
            print( "Exiting." )
            exit( -1 )

def addDarwinPythonFlags( command ):
    command = command.replace(
        "CPPFLAGS=\"-IINSTALLATION_DIR/include",
        "CPPFLAGS=\"-IINSTALLATION_DIR/include "
        "-I$(brew --prefix zlib)/include "
        "-I$(brew --prefix openssl-devel)"
    )
    command = command.replace(
        "LDFLAGS=\"-LINSTALLATION_DIR/lib -Wl,-rpath,INSTALLATION_DIR/lib",
        "LDFLAGS=\"-LINSTALLATION_DIR/lib -Wl,-rpath,INSTALLATION_DIR/lib "
        "-L$(brew --prefix zlib)/lib  -L$(brew --prefix openssl)/lib"
    )
    return command;


def unpack( tarball ):
    extension = os.path.splitext( tarball )
    print(*extension)
    
    command = "tar -xf "
    if ( extension[-1] == ".xz" or extension[-1] == ".gz" ):
        command = "tar -xvf "
    command += tarball

    command += " -C " + BuildDir

    print( "Unpacking " + tarball )
    print( "  " + command )
    subprocess.call( [ command ], shell=True )

def formatBuildCommand( command, lib ):
    system = distro.id()
    if ( system == 'darwin' ):
        sqliteLib = 'libsqlite3.dylib'
        if ( lib == 'glew' ):
            command = command.replace( "-i", "-i \'\'")
        if ( lib == 'Python'):
            checkDarwinPythonDeps()
            command = addDarwinPythonFlags( command )
    else:
        sqliteLib = 'libsqlite3.so'
        if ( system == 'centos' ):
            if ( lib == 'Python' ):
                checkCentOSPythonDeps()
        if ( system == 'ubuntu' ):
            if ( lib == 'Python' ):
                checkUbuntuPythonDeps()

    command = command.replace( "CMAKE_EXE"       , CMake )
    command = command.replace( "MAKE_EXE"        , Make )
    command = command.replace( "C_COMPILER"      , CCompiler )
    command = command.replace( "CPP_COMPILER"    , CppCompiler )
    command = command.replace( "INSTALLATION_DIR", InstallationDir )
    command = command.replace( "QT_VERSION", QtVersion )

    if ( lib == "proj" ):
        sqlitePath = glob.glob( InstallationDir + "/lib/" + sqliteLib )[0]
        command = command.replace( "SQLITE_LIBRARY", sqlitePath )

    return command

def buildLibrary( lib, command ):
    global BuildDir
    buildDir = BuildDir
   # os.makedirs( buildDir, exist_ok=True )

    files = glob.glob( lib+"*" )
    print( lib )
    print( files )
    # We don't want zip, exe, or dos files, just tar
    if ( len(files) > 0 ):
        tarball = ""
        for f in files:
            if ( f.find( "zip" ) == -1 and \
                 f.find( "exe" ) == -1 and \
                 f.find( "dos" ) == -1 ):
                if ( f.find( "tar" ) != -1 ):
                    #libdir = f
                    tarball = f
                    break

        unpack( tarball )
     
        libDir = os.path.splitext( tarball )[0]
        
        # Remove .tar if it wasn't removed from libDir above
        if ( os.path.splitext( libDir )[-1] == '.tar' ):
            libDir = os.path.splitext( libDir )[0]
        buildDir += libDir
    else:
        os.makedirs( lib, exist_ok=True )
        buildDir += lib
    
    if ( command.find( "CMAKE" ) != -1 ):
        buildDir += r'/build'
        os.makedirs( buildDir, exist_ok=True )

    command = formatBuildCommand( command, lib )

    print ("buildDir: " + buildDir )
    print ("command:  " + command )
   
    os.makedirs( buildDir, exist_ok=True )
    
    f = open( buildDir + r'/' + lib + ".txt", "w")
    f.flush()
    p = subprocess.Popen( command, cwd=buildDir, stdout=f, stderr=f, shell=True )
    output = p.communicate()  # this makes us wait for Popen to finish
    f.close()

#
# Argument Parser
#
parser = argparse.ArgumentParser()
parser.add_argument(
    '--installationDir',
    type=str,
    required=False,
    help='Specifies the directory to install libraries into.'
)
parser.add_argument(
    '--cmake',
    type=str,
    required=False,
    help='Specifies the cmake executable.'
)
parser.add_argument(
    '--make',
    type=str,
    required=False,
    help='Specifies the make executable.'
)
parser.add_argument(
    '--CC',
    type=str,
    required=False,
    help='Specifies the C compiler.'
)
parser.add_argument(
    '--CXX',
    type=str,
    required=False,
    help='Specifies the C++ compiler.'
)

def main():

    subprocess.call( [ "rm -rf *.txt" ], shell=True )
    subprocess.call( [ "rm -rf qt" ], shell=True )

    os.makedirs( InstallationDir, exist_ok=True )

    for lib, command in Libraries.items():
        buildLibrary( lib, command )

if __name__ == "__main__":
    main();
