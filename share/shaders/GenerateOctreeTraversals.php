<?php

function dec($i) {
	  return $i-1;
}
$dec = 'dec';

$levels = 2;

function PrintLoop($L) {

	echo <<< EOT

for (int y$L = 0; y$L < yEnd$L; y$L++) {
	for (int x$L = 0; x$L < xEnd$L; x$L++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, x$L, y$L, sideID, $L)) {

EOT;
	if ($L > 0) {
		$LN = $L-1;
		echo <<< EOT
			int yEnd$LN = y$L == lDims$L.y-1 ? lDims1.y : (y$L+1)*2;
			int xEnd$LN = x$L == lDims$L.x-1 ? lDims1.x : (x$L+1)*2;
EOT;
		PrintLoop($LN);
	}

	echo <<< EOT
		}
	}
} // End $L


EOT;

}

PrintLoop($levels-1);

?>
