#########################################################
#                         Notes                         #
#########################################################
# 
# This code generates a single function that supports N
# levels with an unrolled loop. We need to support up to
# 12 levels for a 4096 grid and. It turns out, up to 9
# levels, it performs 10% slower than a function that only
# supports N levels but I found that if you generate a
# function that supports 12 levels, if you render a 9 level
# grid, it performs 120% slower.
#
# This code is only here because I don't feel like deleting
# it right now.
#
# Please refer to GenerateBBTraversals.pl
# 
#########################################################
#                    Generate Code                      #
#########################################################

sub IncrementSpacing {
	my ($str, $n) = @_;
	my $tabs = "\t"x$n;
	$str =~ s/^/$tabs/mg;
	return $str;
}

sub Loop {
	my ($L) = @_;
	my $Lup = $L+1;

	my $ret ="
for (int y$L = y$Lup*2; y$L < yEnd$L; y$L++) {
for (int x$L = x$Lup*2; x$L < xEnd$L; x$L++) {
if (IntersectRaySideCellBBoxDirect(origin, dir, x$L, y$L, sideID, $L) || BBLevels <= $L) {
";
	if ($L > 0) {
		my $LN = $L-1;

		$ret .="
	int yEnd$LN = y$L == lDims$L.y-1 ? lDims$LN.y : (y$L+1)*2;
	int xEnd$LN = x$L == lDims$L.x-1 ? lDims$LN.x : (x$L+1)*2;";
		$ret .="\n";


		$ret .= IncrementSpacing(Loop($LN), 1);
	} else {
		$ret .="
	index[slowDim] = y0;
	index[fastDim] = x0;
	if (IsFaceThatPassedBBTheInitialCell(origin, dir, index, side, cellIndex, entranceFace, t1))
		return true;
";

	}

	$ret .= "

}
}
}
";
	return $ret;
}

sub Function {
	my ($levels) = @_;
	my $ret = "bool SearchSideForInitialCellWithOctree(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, out ivec3 cellIndex, out ivec3 entranceFace, out float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);

";

	$ret .= "	ivec2 lDims0";
	for (my $i = 1; $i < $levels; $i++) {
		$ret .= ", lDims$i";
	}
	$ret .= ";\n\n";

	$ret .= "	lDims0 = GetBBoxArrayDimensions(sideID, 0);\n";
	for (my $i = 1; $i < $levels; $i++) {
		$ret .= "	if (BBLevels > $i) lDims$i = GetBBoxArrayDimensions(sideID, $i); else lDims$i = ivec2(1);\n";
	}

	my $L = $levels-1;
	$ret .= "
	int yEnd$L = lDims$L.y;
	int xEnd$L = lDims$L.x;
	int x${levels} = 0;
	int y${levels} = 0;
";

	$ret .= IncrementSpacing(Loop($L), 1);
	$ret .= "
	return false;
}
\n";

	return $ret;
};


print Function(12);
