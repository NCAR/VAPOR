#!/bin/csh -f

set data = /glade/p/DASG/VAPOR/Data/Raw/checker128
set dataconst = /glade/p/DASG/VAPOR/Data/Raw/constant
set data64 = /glade/p/DASG/VAPOR/Data/Raw/checker64

#./vdc3test.csh -dim 512x512x255 -- -- -lod -1 -level -1 $dataconst

#
# DIM
#
./vdc3test.csh -dim 64x64x64 -- -- $data64
if ($status) exit 1

./vdc3test.csh -dim 128x128x128 -- -- $data
if ($status) exit 1

./vdc3test.csh -dim 120x128x128 -- -- $data
if ($status) exit 1

./vdc3test.csh -dim 128x120x128 -- -- $data
if ($status) exit 1

./vdc3test.csh -dim 128x128x120 -- -- $data
if ($status) exit 1

./vdc3test.csh -dim 120x63x120 -- -- $data
if ($status) exit 1

./vdc3test.csh -dim 127x94x33 -- -- $data
if ($status) exit 1


#
# LOD
#
./vdc3test.csh -dim 128x128x128 -- -lod 0 -- -lod 0 $data
if ($status) exit 1
./vdc3test.csh -dim 128x128x128 -- -lod 1 -- -lod 1 $data
if ($status) exit 1

./vdc3test.csh -dim 128x128x128 -- -- -lod 0 $data
if ($status) exit 1
./vdc3test.csh -dim 128x128x128 -- -- -lod 1 $data
if ($status) exit 1

#
# LEVEL
#
./vdc3test.csh -dim 128x128x128 -- -- -level 0 $data
if ($status) exit 1
./vdc3test.csh -dim 128x128x128 -- -- -level 1 $data
if ($status) exit 1

#
# LEVEL+LOD
#
./vdc3test.csh -dim 128x128x128 -- -- -lod 2 -level 0 $data
if ($status) exit 1
./vdc3test.csh -dim 128x128x128 -- -- -lod 2 -level 1 $data
if ($status) exit 1

#
# BS
#
./vdc3test.csh -cratio 1:10:100:300 -dim 128x128x128 -bs 65x65x65 -- -- $data
if ($status) exit 1

#
# appears to be broken in raw2vdf
#
#./vdc3test.csh -cratio 1:10:100:300 -dim 121x119x134 -bs 65x66x67 -- -- $data
#if ($status) exit 1

#
# CRATIO
#
./vdc3test.csh -dim 128x128x63 -cratio 1 -- -- -lod -1 -level -1 $data
if ($status) exit 1

./vdc3test.csh -dim 128x128x63 -cratio 1 -- -- -lod -1 -level -1 $data
if ($status) exit 1

./vdc3test.csh -dim 128x128x128 -cratio 10:50 -- -- -lod 1 $data
if ($status) exit 1

# broken in 2.x
#./vdc3test.csh -dim 128x128x128 -cratio 10:50 -- -- $data
#if ($status) exit 1


#
# region
#
./vdc3test.csh -dim 128x128x128 -- -- -xregion 5:123 $data
if ($status) exit 1
./vdc3test.csh -dim 128x128x128 -- -- -yregion 5:123 $data
if ($status) exit 1
./vdc3test.csh -dim 128x128x128 -- -- -zregion 5:123 $data
if ($status) exit 1
./vdc3test.csh -dim 128x128x128 -- -- -xregion 90:100 -yregion 5:123 -zregion 10:63  $data
if ($status) exit 1

#
# TS, NUMTS
#
./vdc3test.csh -dim 128x128x128 -numts 10 -- -ts 9 -- -ts 9 $data
if ($status) exit 1


##
## 2D DATA
##


#
# VAR2DXY
#
./vdc3test.csh -dim 128x128x128 -vars2dxy var2dxy -- -var var2dxy -- -var var2dxy $data
if ($status) exit 1

./vdc3test.csh -dim 128x128x128 -vars2dxy var2dxy -- -var var2dxy -- -var var2dxy -xregion 5:123 -yregion 5:50 $data
if ($status) exit 1
