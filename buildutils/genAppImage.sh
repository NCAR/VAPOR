#! /bin/bash

set -x
VAPOR="VAPOR3-$1-Linux"
SRC_DIR=$2

####
# Install vapor into build dir for modification into an AppImage
####

mkdir -p $VAPOR
rm -rf $VAPOR/*
./$VAPOR.sh --skip-license --prefix=$VAPOR

cp ${SRC_DIR}/buildutils/AppRun $VAPOR
cp ${SRC_DIR}/buildutils/vapor.desktop $VAPOR
cp ${SRC_DIR}/share/images/VAPOR.png $VAPOR

####
# Produces a self-contained AppDir with Qt
####

linuxdeployqt=linuxdeployqt-continuous-x86_64.AppImage
rm $linuxdeployqt
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/$linuxdeployqt
chmod 755 $linuxdeployqt

THIRD_PARTY_DIR=/usr/local/VAPOR-Deps/current;\
Qt5_DIR=$THIRD_PARTY_DIR/lib/cmake/Qt5;\
LD_LIBRARY_PATH=$THIRD_PARTY_DIR/lib:\
/opt/intel/oneapi/mpi/latest/lib:\
$THIRD_PARTY_DIR/lib/python3.9/site-packages/Pillow.libs:\
$THIRD_PARTY_DIR/lib/python3.9/site-packages/scipy.libs:\
$THIRD_PARTY_DIR/lib/python3.9/site-packages/numpy.libs \
./$linuxdeployqt \
$VAPOR/bin/vapor \
-qmake=$THIRD_PARTY_DIR/bin/qmake \
-appimage

####
# Produces an AppImage from an AppDir
####

appimagetool=appimagetool-x86_64.AppImage
rm $appimagetool
wget https://github.com/AppImage/appimagetool/releases/download/continuous/$appimagetool
chmod 755 $appimagetool
./$appimagetool $VAPOR
