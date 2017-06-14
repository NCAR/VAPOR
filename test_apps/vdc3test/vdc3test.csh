#!/bin/csh -f

set TmpDir = "/tmp"

if ($#argv < 1) then
	echo "Usage $0 [xcreate options] -- [raw2x options] -- [x2raw options] rawfile"
	exit 1
endif

set ProgName = `basename $0`
set state = 0
set xcreate_options = ""
set raw2x_options = ""
set x2raw_options = ""

while ($#argv > 1) 

	if ("$argv[1]" == "--") then
		@ state += 1
	else 
		switch ("$state")
		case "0":
			set xcreate_options = ($xcreate_options $argv[1])
			breaksw
		case "1":
			set raw2x_options = ($raw2x_options $argv[1])
			breaksw
		case "2":
			set x2raw_options = ($x2raw_options $argv[1])
			breaksw
		default:		
			echo "BOGUS"
			exit 1
			breaksw
		endsw
	endif
	shift
end

set rawfile = $argv[1]

#echo xcreate_options = $xcreate_options
#echo raw2x_options = $raw2x_options
#echo x2raw_options = $x2raw_options
#echo rawfile = $rawfile
#exit 0

set vdffile = $TmpDir/${ProgName:r}_vdf.vdf
/bin/rm -fr $vdffile
/bin/rm -fr ${vdffile:r}_data
set cmd = "vdfcreate -vdc2 $xcreate_options $vdffile"
echo $cmd
$cmd
if ($status != 0) then
	echo "FAILED : $cmd"
	exit 1
endif

set cmd = "raw2vdf $raw2x_options $vdffile $rawfile"
echo $cmd
$cmd
if ($status != 0) then
	echo "FAILED : $cmd"
	exit 1
endif

set vdfrfile = $TmpDir/${ProgName:r}_vdfr.raw
set cmd = "vdf2raw $x2raw_options $vdffile $vdfrfile"
echo $cmd
$cmd
if ($status != 0) then
	echo "FAILED : $cmd"
	exit 1
endif

set master = $TmpDir/${ProgName:r}_nc.nc
/bin/rm -fr $master
/bin/rm -fr ${master:r}_data
set cmd = "vdccreate $xcreate_options $master"
echo $cmd
$cmd
if ($status != 0) then
	echo "FAILED : $cmd"
	exit 1
endif

set cmd = "raw2vdc $raw2x_options $master $rawfile"
echo $cmd
$cmd
if ($status != 0) then
	echo "FAILED : $cmd"
	exit 1
endif

set vdcrfile = $TmpDir/${ProgName:r}_vdcr.raw
set cmd = "vdc2raw $x2raw_options $master $vdcrfile"
echo $cmd
$cmd
if ($status != 0) then
	echo "FAILED : $cmd"
	exit 1
endif

set cmd = "diff $vdfrfile $vdcrfile"
echo $cmd
$cmd
if ($status != 0) then
	echo "FAILED : $cmd"
	exit 1
endif

echo "SUCCESS"
/bin/rm $vdfrfile $vdcrfile
exit 0
