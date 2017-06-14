#!/bin/csh -f


set wasp2raw = ../../targets/Darwin_x86_64/bin/wasp2raw
set raw2wasp = ../../targets/Darwin_x86_64/bin/raw2wasp
set vdf2raw = ../../targets/Darwin_x86_64/bin/vdf2raw
set raw2vdf = ../../targets/Darwin_x86_64/bin/raw2vdf

set rawfile = /tmp/marsch256
set var = marsch
set vdffile = /tmp/marsch256.vdf
set waspfile = /tmp/marsch.nc0


echo $raw2vdf -var $var $vdffile $rawfile
$raw2vdf -var $var $vdffile $rawfile

echo $raw2wasp -var $var $waspfile $rawfile
$raw2wasp -var $var $waspfile $rawfile

set vdftmp = /tmp/vdftmp.$$.raw
set wasptmp = /tmp/wasptmp.$$.raw

foreach lod (0 1 2 3)
	foreach level (0 1 2 3)

		echo "lod = $lod, level = $level"

		echo $vdf2raw -var $var -lod $lod -level $level $vdffile $vdftmp
		$vdf2raw -var $var -lod $lod -level $level $vdffile $vdftmp
		if ($status != 0) exit(1);

		echo $wasp2raw -var $var -lod $lod -level $level $waspfile $wasptmp
		$wasp2raw -var $var -lod $lod -level $level $waspfile $wasptmp
		if ($status != 0) exit(1);

		diff $vdftmp $wasptmp
		set mystatus = $status
		/bin/rm -f $wasptmp  $vdftmp

		if ($mystatus != 0) exit(1);
	end
end







set rawfile = /tmp/checker96
set var = checker
set vdffile = /tmp/checker96.vdf
set waspfile = /tmp/checker.nc0


echo $raw2vdf -var $var $vdffile $rawfile
$raw2vdf -var $var $vdffile $rawfile

echo $raw2wasp -var $var $waspfile $rawfile
$raw2wasp -var $var $waspfile $rawfile

set vdftmp = /tmp/vdftmp.$$.raw
set wasptmp = /tmp/wasptmp.$$.raw

foreach lod (0 1 2 3)
	foreach level (0 1 2 3)

		echo "lod = $lod, level = $level"

		echo $vdf2raw -var $var -lod $lod -level $level $vdffile $vdftmp
		$vdf2raw -var $var -lod $lod -level $level $vdffile $vdftmp
		if ($status != 0) exit(1);

		echo $wasp2raw -var $var -lod $lod -level $level $waspfile $wasptmp
		$wasp2raw -var $var -lod $lod -level $level $waspfile $wasptmp
		if ($status != 0) exit(1);

		diff $vdftmp $wasptmp
		set mystatus = $status
		/bin/rm -f $wasptmp  $vdftmp

		if ($mystatus != 0) exit(1);
	end
end
