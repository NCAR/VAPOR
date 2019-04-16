#########################################################
#                         Notes                         #
#########################################################
# 
# This code generates functions with the same signature and functionality as
# SearchSideForInitialCell but using the BB traversal
# The functions will be called SearchSideForInitialCellWithOctree_{1-8}Levels
# 
# Below there is a parametric non-recursive function but it is very slow in GLSL
# 
#
#########################################################
#              Example 2 Level Traversal                #
#########################################################
# 
# bool SearchSideForInitialCellWithOctree_2Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, OUT float t1)
# {
#     ivec3 side = GetFaceFromFaceIndex(sideID);
#     ivec3 index = (side+1)/2 * (cellDims-1);
#     
#     ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
#     ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
#     
#     for (int y1 = 0; y1 < lDims1.y; y1++) {
#         for (int x1 = 0; x1 < lDims1.x; x1++) {
#             if (IntersectRaySideCellBBoxDirect(origin, dir, x1, y1, sideID, 1)) {
#                 int y0End = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
#                 int x0End = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
#                 
#                 for (int y0 = y1*2; y0 < y0End; y0++) {
#                     for (int x0 = x1*2; x0 < lDims0.x; x0++) {
#                         if (IntersectRaySideCellBBoxDirect(origin, dir, x0, y0, sideID, 0)) {
#                             index[slowDim] = y0;
#                             index[fastDim] = x0;
#                             if (IntersectRayCellFace(origin, dir, index, side, t1)) {
#                                 if (IsRayEnteringCell(dir, index, side)) {
#                                     cellIndex = index;
#                                     entranceFace = side;
#                                     return true;
#                                 }
#                             }
#                             
#                         }
#                     }
#                 }
#             }
#         }
#     }
#     return false;
# }
# 
#########################################################
#               Non-Recursive Traversal                 #
#########################################################
# 
# bool SearchSideForInitialCellWithOctreeParametric(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, OUT float t1)
# {
#     ivec3 side = GetFaceFromFaceIndex(sideID);
#     ivec3 index = (side+1)/2 * (cellDims-1);
#     
#     #define MAX_LEVELS 8
#     int x[MAX_LEVELS];
#     int y[MAX_LEVELS];
#     int xEnd[MAX_LEVELS];
#     int yEnd[MAX_LEVELS];
#     ivec2 dims[MAX_LEVELS];
#     
#     for (int i = 0; i < BBLevels; i++)
#     dims[i] = GetBBoxArrayDimensions(sideID, i);
#     
#     
#     int L = min(BBLevels-1, MAX_LEVELS-1);
#     int dL = 0;
#     
#     xEnd[L] = dims[L].x;
#     yEnd[L] = dims[L].y;
#     
#     while (true) {
#         for (y[L] = 0; y[L] < yEnd[L] && dL==0; y[L]++) {
#             for (x[L] = 0; x[L] < xEnd[L] && dL==0; x[L]++) {
#                 if (IntersectRaySideCellBBoxDirect(origin, dir, x[L], y[L], sideID, L)) {
#                     if (L == 0) {
#                         index[slowDim] = y[L];
#                         index[fastDim] = x[L];
#                         if (IntersectRayCellFace(origin, dir, index, side, t1)) {
#                             if (IsRayEnteringCell(dir, index, side)) {
#                                 cellIndex = index;
#                                 entranceFace = side;
#                                 return true;
#                             }
#                         }
#                     } else {
#                         dL = -1;
#                         xEnd[L-1] = x[L] == dims[L].x-1 ? dims[L-1].x : (x[L]+1)*2;
#                         yEnd[L-1] = y[L] == dims[L].y-1 ? dims[L-1].y : (y[L]+1)*2;
#                     }
#                 }
#             }
#         }
#         if (dL == -1) {
#             L--;
#             dL = 0;
#         } else {
#             break;
#         }
#     }
#     return false;
# }
# 
# 
# 
# 
# 
# 
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
// for (int y$L = y$Lup*2; y$L < yEnd$L; y$L++) {
int y$L = y$Lup*2;
for (int x$L = x$Lup*2; x$L < xEnd$L && y$L < yEnd$L; x$L++) {
if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x$L, y$L, sideID, $L)) {
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
	if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
		intersections++;
";

	}

	$ret .= "
}
if (x$L == xEnd$L-1) {
	x$L = x$Lup*2-1;
	y$L++;
}
}
";
	return $ret;
}

sub Function {
	my ($levels) = @_;
	my $ret =
"int SearchSideForInitialCellWithOctree_${levels}Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


";


	for (my $i = 0; $i < $levels; $i++) {
		$ret .= "	ivec2 lDims$i = GetBBoxArrayDimensions(sideID, $i);\n"
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
	return intersections;
}
\n";

	return $ret;
};


for (my $i = 1; $i <= 12; $i++) {
	print Function($i);
}
