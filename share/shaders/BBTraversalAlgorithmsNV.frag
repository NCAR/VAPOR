int SearchSideForInitialCellWithOctree_1Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);

	int yEnd0 = lDims0.y;
	int xEnd0 = lDims0.x;
	int x1 = 0;
	int y1 = 0;
	
	// for (int y0 = y1*2; y0 < yEnd0; y0++) {
	int y0 = y1*2;
	for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
	
		index[slowDim] = y0;
		index[fastDim] = x0;
		if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
			intersections++;
	
	}
	if (x0 == xEnd0-1) {
		x0 = x1*2-1;
		y0++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_2Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);

	int yEnd1 = lDims1.y;
	int xEnd1 = lDims1.x;
	int x2 = 0;
	int y2 = 0;
	
	// for (int y1 = y2*2; y1 < yEnd1; y1++) {
	int y1 = y2*2;
	for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
	
		int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
		int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
		
		// for (int y0 = y1*2; y0 < yEnd0; y0++) {
		int y0 = y1*2;
		for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
		
			index[slowDim] = y0;
			index[fastDim] = x0;
			if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
				intersections++;
		
		}
		if (x0 == xEnd0-1) {
			x0 = x1*2-1;
			y0++;
		}
		}
	
	}
	if (x1 == xEnd1-1) {
		x1 = x2*2-1;
		y1++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_3Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);

	int yEnd2 = lDims2.y;
	int xEnd2 = lDims2.x;
	int x3 = 0;
	int y3 = 0;
	
	// for (int y2 = y3*2; y2 < yEnd2; y2++) {
	int y2 = y3*2;
	for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
	
		int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
		int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
		
		// for (int y1 = y2*2; y1 < yEnd1; y1++) {
		int y1 = y2*2;
		for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
		
			int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
			int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
			
			// for (int y0 = y1*2; y0 < yEnd0; y0++) {
			int y0 = y1*2;
			for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
			
				index[slowDim] = y0;
				index[fastDim] = x0;
				if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
					intersections++;
			
			}
			if (x0 == xEnd0-1) {
				x0 = x1*2-1;
				y0++;
			}
			}
		
		}
		if (x1 == xEnd1-1) {
			x1 = x2*2-1;
			y1++;
		}
		}
	
	}
	if (x2 == xEnd2-1) {
		x2 = x3*2-1;
		y2++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_4Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);
	ivec2 lDims3 = GetBBoxArrayDimensions(sideID, 3);

	int yEnd3 = lDims3.y;
	int xEnd3 = lDims3.x;
	int x4 = 0;
	int y4 = 0;
	
	// for (int y3 = y4*2; y3 < yEnd3; y3++) {
	int y3 = y4*2;
	for (int x3 = x4*2; x3 < xEnd3 && y3 < yEnd3; x3++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x3, y3, sideID, 3)) {
	
		int yEnd2 = y3 == lDims3.y-1 ? lDims2.y : (y3+1)*2;
		int xEnd2 = x3 == lDims3.x-1 ? lDims2.x : (x3+1)*2;
		
		// for (int y2 = y3*2; y2 < yEnd2; y2++) {
		int y2 = y3*2;
		for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
		
			int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
			int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
			
			// for (int y1 = y2*2; y1 < yEnd1; y1++) {
			int y1 = y2*2;
			for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
			
				int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
				int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
				
				// for (int y0 = y1*2; y0 < yEnd0; y0++) {
				int y0 = y1*2;
				for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
				if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
				
					index[slowDim] = y0;
					index[fastDim] = x0;
					if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
						intersections++;
				
				}
				if (x0 == xEnd0-1) {
					x0 = x1*2-1;
					y0++;
				}
				}
			
			}
			if (x1 == xEnd1-1) {
				x1 = x2*2-1;
				y1++;
			}
			}
		
		}
		if (x2 == xEnd2-1) {
			x2 = x3*2-1;
			y2++;
		}
		}
	
	}
	if (x3 == xEnd3-1) {
		x3 = x4*2-1;
		y3++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_5Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);
	ivec2 lDims3 = GetBBoxArrayDimensions(sideID, 3);
	ivec2 lDims4 = GetBBoxArrayDimensions(sideID, 4);

	int yEnd4 = lDims4.y;
	int xEnd4 = lDims4.x;
	int x5 = 0;
	int y5 = 0;
	
	// for (int y4 = y5*2; y4 < yEnd4; y4++) {
	int y4 = y5*2;
	for (int x4 = x5*2; x4 < xEnd4 && y4 < yEnd4; x4++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x4, y4, sideID, 4)) {
	
		int yEnd3 = y4 == lDims4.y-1 ? lDims3.y : (y4+1)*2;
		int xEnd3 = x4 == lDims4.x-1 ? lDims3.x : (x4+1)*2;
		
		// for (int y3 = y4*2; y3 < yEnd3; y3++) {
		int y3 = y4*2;
		for (int x3 = x4*2; x3 < xEnd3 && y3 < yEnd3; x3++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x3, y3, sideID, 3)) {
		
			int yEnd2 = y3 == lDims3.y-1 ? lDims2.y : (y3+1)*2;
			int xEnd2 = x3 == lDims3.x-1 ? lDims2.x : (x3+1)*2;
			
			// for (int y2 = y3*2; y2 < yEnd2; y2++) {
			int y2 = y3*2;
			for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
			
				int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
				int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
				
				// for (int y1 = y2*2; y1 < yEnd1; y1++) {
				int y1 = y2*2;
				for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
				if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
				
					int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
					int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
					
					// for (int y0 = y1*2; y0 < yEnd0; y0++) {
					int y0 = y1*2;
					for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
					if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
					
						index[slowDim] = y0;
						index[fastDim] = x0;
						if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
							intersections++;
					
					}
					if (x0 == xEnd0-1) {
						x0 = x1*2-1;
						y0++;
					}
					}
				
				}
				if (x1 == xEnd1-1) {
					x1 = x2*2-1;
					y1++;
				}
				}
			
			}
			if (x2 == xEnd2-1) {
				x2 = x3*2-1;
				y2++;
			}
			}
		
		}
		if (x3 == xEnd3-1) {
			x3 = x4*2-1;
			y3++;
		}
		}
	
	}
	if (x4 == xEnd4-1) {
		x4 = x5*2-1;
		y4++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_6Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);
	ivec2 lDims3 = GetBBoxArrayDimensions(sideID, 3);
	ivec2 lDims4 = GetBBoxArrayDimensions(sideID, 4);
	ivec2 lDims5 = GetBBoxArrayDimensions(sideID, 5);

	int yEnd5 = lDims5.y;
	int xEnd5 = lDims5.x;
	int x6 = 0;
	int y6 = 0;
	
	// for (int y5 = y6*2; y5 < yEnd5; y5++) {
	int y5 = y6*2;
	for (int x5 = x6*2; x5 < xEnd5 && y5 < yEnd5; x5++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x5, y5, sideID, 5)) {
	
		int yEnd4 = y5 == lDims5.y-1 ? lDims4.y : (y5+1)*2;
		int xEnd4 = x5 == lDims5.x-1 ? lDims4.x : (x5+1)*2;
		
		// for (int y4 = y5*2; y4 < yEnd4; y4++) {
		int y4 = y5*2;
		for (int x4 = x5*2; x4 < xEnd4 && y4 < yEnd4; x4++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x4, y4, sideID, 4)) {
		
			int yEnd3 = y4 == lDims4.y-1 ? lDims3.y : (y4+1)*2;
			int xEnd3 = x4 == lDims4.x-1 ? lDims3.x : (x4+1)*2;
			
			// for (int y3 = y4*2; y3 < yEnd3; y3++) {
			int y3 = y4*2;
			for (int x3 = x4*2; x3 < xEnd3 && y3 < yEnd3; x3++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x3, y3, sideID, 3)) {
			
				int yEnd2 = y3 == lDims3.y-1 ? lDims2.y : (y3+1)*2;
				int xEnd2 = x3 == lDims3.x-1 ? lDims2.x : (x3+1)*2;
				
				// for (int y2 = y3*2; y2 < yEnd2; y2++) {
				int y2 = y3*2;
				for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
				if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
				
					int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
					int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
					
					// for (int y1 = y2*2; y1 < yEnd1; y1++) {
					int y1 = y2*2;
					for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
					if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
					
						int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
						int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
						
						// for (int y0 = y1*2; y0 < yEnd0; y0++) {
						int y0 = y1*2;
						for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
						if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
						
							index[slowDim] = y0;
							index[fastDim] = x0;
							if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
								intersections++;
						
						}
						if (x0 == xEnd0-1) {
							x0 = x1*2-1;
							y0++;
						}
						}
					
					}
					if (x1 == xEnd1-1) {
						x1 = x2*2-1;
						y1++;
					}
					}
				
				}
				if (x2 == xEnd2-1) {
					x2 = x3*2-1;
					y2++;
				}
				}
			
			}
			if (x3 == xEnd3-1) {
				x3 = x4*2-1;
				y3++;
			}
			}
		
		}
		if (x4 == xEnd4-1) {
			x4 = x5*2-1;
			y4++;
		}
		}
	
	}
	if (x5 == xEnd5-1) {
		x5 = x6*2-1;
		y5++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_7Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);
	ivec2 lDims3 = GetBBoxArrayDimensions(sideID, 3);
	ivec2 lDims4 = GetBBoxArrayDimensions(sideID, 4);
	ivec2 lDims5 = GetBBoxArrayDimensions(sideID, 5);
	ivec2 lDims6 = GetBBoxArrayDimensions(sideID, 6);

	int yEnd6 = lDims6.y;
	int xEnd6 = lDims6.x;
	int x7 = 0;
	int y7 = 0;
	
	// for (int y6 = y7*2; y6 < yEnd6; y6++) {
	int y6 = y7*2;
	for (int x6 = x7*2; x6 < xEnd6 && y6 < yEnd6; x6++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x6, y6, sideID, 6)) {
	
		int yEnd5 = y6 == lDims6.y-1 ? lDims5.y : (y6+1)*2;
		int xEnd5 = x6 == lDims6.x-1 ? lDims5.x : (x6+1)*2;
		
		// for (int y5 = y6*2; y5 < yEnd5; y5++) {
		int y5 = y6*2;
		for (int x5 = x6*2; x5 < xEnd5 && y5 < yEnd5; x5++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x5, y5, sideID, 5)) {
		
			int yEnd4 = y5 == lDims5.y-1 ? lDims4.y : (y5+1)*2;
			int xEnd4 = x5 == lDims5.x-1 ? lDims4.x : (x5+1)*2;
			
			// for (int y4 = y5*2; y4 < yEnd4; y4++) {
			int y4 = y5*2;
			for (int x4 = x5*2; x4 < xEnd4 && y4 < yEnd4; x4++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x4, y4, sideID, 4)) {
			
				int yEnd3 = y4 == lDims4.y-1 ? lDims3.y : (y4+1)*2;
				int xEnd3 = x4 == lDims4.x-1 ? lDims3.x : (x4+1)*2;
				
				// for (int y3 = y4*2; y3 < yEnd3; y3++) {
				int y3 = y4*2;
				for (int x3 = x4*2; x3 < xEnd3 && y3 < yEnd3; x3++) {
				if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x3, y3, sideID, 3)) {
				
					int yEnd2 = y3 == lDims3.y-1 ? lDims2.y : (y3+1)*2;
					int xEnd2 = x3 == lDims3.x-1 ? lDims2.x : (x3+1)*2;
					
					// for (int y2 = y3*2; y2 < yEnd2; y2++) {
					int y2 = y3*2;
					for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
					if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
					
						int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
						int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
						
						// for (int y1 = y2*2; y1 < yEnd1; y1++) {
						int y1 = y2*2;
						for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
						if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
						
							int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
							int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
							
							// for (int y0 = y1*2; y0 < yEnd0; y0++) {
							int y0 = y1*2;
							for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
							if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
							
								index[slowDim] = y0;
								index[fastDim] = x0;
								if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
									intersections++;
							
							}
							if (x0 == xEnd0-1) {
								x0 = x1*2-1;
								y0++;
							}
							}
						
						}
						if (x1 == xEnd1-1) {
							x1 = x2*2-1;
							y1++;
						}
						}
					
					}
					if (x2 == xEnd2-1) {
						x2 = x3*2-1;
						y2++;
					}
					}
				
				}
				if (x3 == xEnd3-1) {
					x3 = x4*2-1;
					y3++;
				}
				}
			
			}
			if (x4 == xEnd4-1) {
				x4 = x5*2-1;
				y4++;
			}
			}
		
		}
		if (x5 == xEnd5-1) {
			x5 = x6*2-1;
			y5++;
		}
		}
	
	}
	if (x6 == xEnd6-1) {
		x6 = x7*2-1;
		y6++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_8Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);
	ivec2 lDims3 = GetBBoxArrayDimensions(sideID, 3);
	ivec2 lDims4 = GetBBoxArrayDimensions(sideID, 4);
	ivec2 lDims5 = GetBBoxArrayDimensions(sideID, 5);
	ivec2 lDims6 = GetBBoxArrayDimensions(sideID, 6);
	ivec2 lDims7 = GetBBoxArrayDimensions(sideID, 7);

	int yEnd7 = lDims7.y;
	int xEnd7 = lDims7.x;
	int x8 = 0;
	int y8 = 0;
	
	// for (int y7 = y8*2; y7 < yEnd7; y7++) {
	int y7 = y8*2;
	for (int x7 = x8*2; x7 < xEnd7 && y7 < yEnd7; x7++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x7, y7, sideID, 7)) {
	
		int yEnd6 = y7 == lDims7.y-1 ? lDims6.y : (y7+1)*2;
		int xEnd6 = x7 == lDims7.x-1 ? lDims6.x : (x7+1)*2;
		
		// for (int y6 = y7*2; y6 < yEnd6; y6++) {
		int y6 = y7*2;
		for (int x6 = x7*2; x6 < xEnd6 && y6 < yEnd6; x6++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x6, y6, sideID, 6)) {
		
			int yEnd5 = y6 == lDims6.y-1 ? lDims5.y : (y6+1)*2;
			int xEnd5 = x6 == lDims6.x-1 ? lDims5.x : (x6+1)*2;
			
			// for (int y5 = y6*2; y5 < yEnd5; y5++) {
			int y5 = y6*2;
			for (int x5 = x6*2; x5 < xEnd5 && y5 < yEnd5; x5++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x5, y5, sideID, 5)) {
			
				int yEnd4 = y5 == lDims5.y-1 ? lDims4.y : (y5+1)*2;
				int xEnd4 = x5 == lDims5.x-1 ? lDims4.x : (x5+1)*2;
				
				// for (int y4 = y5*2; y4 < yEnd4; y4++) {
				int y4 = y5*2;
				for (int x4 = x5*2; x4 < xEnd4 && y4 < yEnd4; x4++) {
				if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x4, y4, sideID, 4)) {
				
					int yEnd3 = y4 == lDims4.y-1 ? lDims3.y : (y4+1)*2;
					int xEnd3 = x4 == lDims4.x-1 ? lDims3.x : (x4+1)*2;
					
					// for (int y3 = y4*2; y3 < yEnd3; y3++) {
					int y3 = y4*2;
					for (int x3 = x4*2; x3 < xEnd3 && y3 < yEnd3; x3++) {
					if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x3, y3, sideID, 3)) {
					
						int yEnd2 = y3 == lDims3.y-1 ? lDims2.y : (y3+1)*2;
						int xEnd2 = x3 == lDims3.x-1 ? lDims2.x : (x3+1)*2;
						
						// for (int y2 = y3*2; y2 < yEnd2; y2++) {
						int y2 = y3*2;
						for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
						if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
						
							int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
							int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
							
							// for (int y1 = y2*2; y1 < yEnd1; y1++) {
							int y1 = y2*2;
							for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
							if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
							
								int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
								int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
								
								// for (int y0 = y1*2; y0 < yEnd0; y0++) {
								int y0 = y1*2;
								for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
								if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
								
									index[slowDim] = y0;
									index[fastDim] = x0;
									if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
										intersections++;
								
								}
								if (x0 == xEnd0-1) {
									x0 = x1*2-1;
									y0++;
								}
								}
							
							}
							if (x1 == xEnd1-1) {
								x1 = x2*2-1;
								y1++;
							}
							}
						
						}
						if (x2 == xEnd2-1) {
							x2 = x3*2-1;
							y2++;
						}
						}
					
					}
					if (x3 == xEnd3-1) {
						x3 = x4*2-1;
						y3++;
					}
					}
				
				}
				if (x4 == xEnd4-1) {
					x4 = x5*2-1;
					y4++;
				}
				}
			
			}
			if (x5 == xEnd5-1) {
				x5 = x6*2-1;
				y5++;
			}
			}
		
		}
		if (x6 == xEnd6-1) {
			x6 = x7*2-1;
			y6++;
		}
		}
	
	}
	if (x7 == xEnd7-1) {
		x7 = x8*2-1;
		y7++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_9Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);
	ivec2 lDims3 = GetBBoxArrayDimensions(sideID, 3);
	ivec2 lDims4 = GetBBoxArrayDimensions(sideID, 4);
	ivec2 lDims5 = GetBBoxArrayDimensions(sideID, 5);
	ivec2 lDims6 = GetBBoxArrayDimensions(sideID, 6);
	ivec2 lDims7 = GetBBoxArrayDimensions(sideID, 7);
	ivec2 lDims8 = GetBBoxArrayDimensions(sideID, 8);

	int yEnd8 = lDims8.y;
	int xEnd8 = lDims8.x;
	int x9 = 0;
	int y9 = 0;
	
	// for (int y8 = y9*2; y8 < yEnd8; y8++) {
	int y8 = y9*2;
	for (int x8 = x9*2; x8 < xEnd8 && y8 < yEnd8; x8++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x8, y8, sideID, 8)) {
	
		int yEnd7 = y8 == lDims8.y-1 ? lDims7.y : (y8+1)*2;
		int xEnd7 = x8 == lDims8.x-1 ? lDims7.x : (x8+1)*2;
		
		// for (int y7 = y8*2; y7 < yEnd7; y7++) {
		int y7 = y8*2;
		for (int x7 = x8*2; x7 < xEnd7 && y7 < yEnd7; x7++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x7, y7, sideID, 7)) {
		
			int yEnd6 = y7 == lDims7.y-1 ? lDims6.y : (y7+1)*2;
			int xEnd6 = x7 == lDims7.x-1 ? lDims6.x : (x7+1)*2;
			
			// for (int y6 = y7*2; y6 < yEnd6; y6++) {
			int y6 = y7*2;
			for (int x6 = x7*2; x6 < xEnd6 && y6 < yEnd6; x6++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x6, y6, sideID, 6)) {
			
				int yEnd5 = y6 == lDims6.y-1 ? lDims5.y : (y6+1)*2;
				int xEnd5 = x6 == lDims6.x-1 ? lDims5.x : (x6+1)*2;
				
				// for (int y5 = y6*2; y5 < yEnd5; y5++) {
				int y5 = y6*2;
				for (int x5 = x6*2; x5 < xEnd5 && y5 < yEnd5; x5++) {
				if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x5, y5, sideID, 5)) {
				
					int yEnd4 = y5 == lDims5.y-1 ? lDims4.y : (y5+1)*2;
					int xEnd4 = x5 == lDims5.x-1 ? lDims4.x : (x5+1)*2;
					
					// for (int y4 = y5*2; y4 < yEnd4; y4++) {
					int y4 = y5*2;
					for (int x4 = x5*2; x4 < xEnd4 && y4 < yEnd4; x4++) {
					if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x4, y4, sideID, 4)) {
					
						int yEnd3 = y4 == lDims4.y-1 ? lDims3.y : (y4+1)*2;
						int xEnd3 = x4 == lDims4.x-1 ? lDims3.x : (x4+1)*2;
						
						// for (int y3 = y4*2; y3 < yEnd3; y3++) {
						int y3 = y4*2;
						for (int x3 = x4*2; x3 < xEnd3 && y3 < yEnd3; x3++) {
						if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x3, y3, sideID, 3)) {
						
							int yEnd2 = y3 == lDims3.y-1 ? lDims2.y : (y3+1)*2;
							int xEnd2 = x3 == lDims3.x-1 ? lDims2.x : (x3+1)*2;
							
							// for (int y2 = y3*2; y2 < yEnd2; y2++) {
							int y2 = y3*2;
							for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
							if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
							
								int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
								int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
								
								// for (int y1 = y2*2; y1 < yEnd1; y1++) {
								int y1 = y2*2;
								for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
								if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
								
									int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
									int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
									
									// for (int y0 = y1*2; y0 < yEnd0; y0++) {
									int y0 = y1*2;
									for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
									if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
									
										index[slowDim] = y0;
										index[fastDim] = x0;
										if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
											intersections++;
									
									}
									if (x0 == xEnd0-1) {
										x0 = x1*2-1;
										y0++;
									}
									}
								
								}
								if (x1 == xEnd1-1) {
									x1 = x2*2-1;
									y1++;
								}
								}
							
							}
							if (x2 == xEnd2-1) {
								x2 = x3*2-1;
								y2++;
							}
							}
						
						}
						if (x3 == xEnd3-1) {
							x3 = x4*2-1;
							y3++;
						}
						}
					
					}
					if (x4 == xEnd4-1) {
						x4 = x5*2-1;
						y4++;
					}
					}
				
				}
				if (x5 == xEnd5-1) {
					x5 = x6*2-1;
					y5++;
				}
				}
			
			}
			if (x6 == xEnd6-1) {
				x6 = x7*2-1;
				y6++;
			}
			}
		
		}
		if (x7 == xEnd7-1) {
			x7 = x8*2-1;
			y7++;
		}
		}
	
	}
	if (x8 == xEnd8-1) {
		x8 = x9*2-1;
		y8++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_10Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);
	ivec2 lDims3 = GetBBoxArrayDimensions(sideID, 3);
	ivec2 lDims4 = GetBBoxArrayDimensions(sideID, 4);
	ivec2 lDims5 = GetBBoxArrayDimensions(sideID, 5);
	ivec2 lDims6 = GetBBoxArrayDimensions(sideID, 6);
	ivec2 lDims7 = GetBBoxArrayDimensions(sideID, 7);
	ivec2 lDims8 = GetBBoxArrayDimensions(sideID, 8);
	ivec2 lDims9 = GetBBoxArrayDimensions(sideID, 9);

	int yEnd9 = lDims9.y;
	int xEnd9 = lDims9.x;
	int x10 = 0;
	int y10 = 0;
	
	// for (int y9 = y10*2; y9 < yEnd9; y9++) {
	int y9 = y10*2;
	for (int x9 = x10*2; x9 < xEnd9 && y9 < yEnd9; x9++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x9, y9, sideID, 9)) {
	
		int yEnd8 = y9 == lDims9.y-1 ? lDims8.y : (y9+1)*2;
		int xEnd8 = x9 == lDims9.x-1 ? lDims8.x : (x9+1)*2;
		
		// for (int y8 = y9*2; y8 < yEnd8; y8++) {
		int y8 = y9*2;
		for (int x8 = x9*2; x8 < xEnd8 && y8 < yEnd8; x8++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x8, y8, sideID, 8)) {
		
			int yEnd7 = y8 == lDims8.y-1 ? lDims7.y : (y8+1)*2;
			int xEnd7 = x8 == lDims8.x-1 ? lDims7.x : (x8+1)*2;
			
			// for (int y7 = y8*2; y7 < yEnd7; y7++) {
			int y7 = y8*2;
			for (int x7 = x8*2; x7 < xEnd7 && y7 < yEnd7; x7++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x7, y7, sideID, 7)) {
			
				int yEnd6 = y7 == lDims7.y-1 ? lDims6.y : (y7+1)*2;
				int xEnd6 = x7 == lDims7.x-1 ? lDims6.x : (x7+1)*2;
				
				// for (int y6 = y7*2; y6 < yEnd6; y6++) {
				int y6 = y7*2;
				for (int x6 = x7*2; x6 < xEnd6 && y6 < yEnd6; x6++) {
				if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x6, y6, sideID, 6)) {
				
					int yEnd5 = y6 == lDims6.y-1 ? lDims5.y : (y6+1)*2;
					int xEnd5 = x6 == lDims6.x-1 ? lDims5.x : (x6+1)*2;
					
					// for (int y5 = y6*2; y5 < yEnd5; y5++) {
					int y5 = y6*2;
					for (int x5 = x6*2; x5 < xEnd5 && y5 < yEnd5; x5++) {
					if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x5, y5, sideID, 5)) {
					
						int yEnd4 = y5 == lDims5.y-1 ? lDims4.y : (y5+1)*2;
						int xEnd4 = x5 == lDims5.x-1 ? lDims4.x : (x5+1)*2;
						
						// for (int y4 = y5*2; y4 < yEnd4; y4++) {
						int y4 = y5*2;
						for (int x4 = x5*2; x4 < xEnd4 && y4 < yEnd4; x4++) {
						if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x4, y4, sideID, 4)) {
						
							int yEnd3 = y4 == lDims4.y-1 ? lDims3.y : (y4+1)*2;
							int xEnd3 = x4 == lDims4.x-1 ? lDims3.x : (x4+1)*2;
							
							// for (int y3 = y4*2; y3 < yEnd3; y3++) {
							int y3 = y4*2;
							for (int x3 = x4*2; x3 < xEnd3 && y3 < yEnd3; x3++) {
							if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x3, y3, sideID, 3)) {
							
								int yEnd2 = y3 == lDims3.y-1 ? lDims2.y : (y3+1)*2;
								int xEnd2 = x3 == lDims3.x-1 ? lDims2.x : (x3+1)*2;
								
								// for (int y2 = y3*2; y2 < yEnd2; y2++) {
								int y2 = y3*2;
								for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
								if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
								
									int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
									int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
									
									// for (int y1 = y2*2; y1 < yEnd1; y1++) {
									int y1 = y2*2;
									for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
									if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
									
										int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
										int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
										
										// for (int y0 = y1*2; y0 < yEnd0; y0++) {
										int y0 = y1*2;
										for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
										if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
										
											index[slowDim] = y0;
											index[fastDim] = x0;
											if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
												intersections++;
										
										}
										if (x0 == xEnd0-1) {
											x0 = x1*2-1;
											y0++;
										}
										}
									
									}
									if (x1 == xEnd1-1) {
										x1 = x2*2-1;
										y1++;
									}
									}
								
								}
								if (x2 == xEnd2-1) {
									x2 = x3*2-1;
									y2++;
								}
								}
							
							}
							if (x3 == xEnd3-1) {
								x3 = x4*2-1;
								y3++;
							}
							}
						
						}
						if (x4 == xEnd4-1) {
							x4 = x5*2-1;
							y4++;
						}
						}
					
					}
					if (x5 == xEnd5-1) {
						x5 = x6*2-1;
						y5++;
					}
					}
				
				}
				if (x6 == xEnd6-1) {
					x6 = x7*2-1;
					y6++;
				}
				}
			
			}
			if (x7 == xEnd7-1) {
				x7 = x8*2-1;
				y7++;
			}
			}
		
		}
		if (x8 == xEnd8-1) {
			x8 = x9*2-1;
			y8++;
		}
		}
	
	}
	if (x9 == xEnd9-1) {
		x9 = x10*2-1;
		y9++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_11Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);
	ivec2 lDims3 = GetBBoxArrayDimensions(sideID, 3);
	ivec2 lDims4 = GetBBoxArrayDimensions(sideID, 4);
	ivec2 lDims5 = GetBBoxArrayDimensions(sideID, 5);
	ivec2 lDims6 = GetBBoxArrayDimensions(sideID, 6);
	ivec2 lDims7 = GetBBoxArrayDimensions(sideID, 7);
	ivec2 lDims8 = GetBBoxArrayDimensions(sideID, 8);
	ivec2 lDims9 = GetBBoxArrayDimensions(sideID, 9);
	ivec2 lDims10 = GetBBoxArrayDimensions(sideID, 10);

	int yEnd10 = lDims10.y;
	int xEnd10 = lDims10.x;
	int x11 = 0;
	int y11 = 0;
	
	// for (int y10 = y11*2; y10 < yEnd10; y10++) {
	int y10 = y11*2;
	for (int x10 = x11*2; x10 < xEnd10 && y10 < yEnd10; x10++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x10, y10, sideID, 10)) {
	
		int yEnd9 = y10 == lDims10.y-1 ? lDims9.y : (y10+1)*2;
		int xEnd9 = x10 == lDims10.x-1 ? lDims9.x : (x10+1)*2;
		
		// for (int y9 = y10*2; y9 < yEnd9; y9++) {
		int y9 = y10*2;
		for (int x9 = x10*2; x9 < xEnd9 && y9 < yEnd9; x9++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x9, y9, sideID, 9)) {
		
			int yEnd8 = y9 == lDims9.y-1 ? lDims8.y : (y9+1)*2;
			int xEnd8 = x9 == lDims9.x-1 ? lDims8.x : (x9+1)*2;
			
			// for (int y8 = y9*2; y8 < yEnd8; y8++) {
			int y8 = y9*2;
			for (int x8 = x9*2; x8 < xEnd8 && y8 < yEnd8; x8++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x8, y8, sideID, 8)) {
			
				int yEnd7 = y8 == lDims8.y-1 ? lDims7.y : (y8+1)*2;
				int xEnd7 = x8 == lDims8.x-1 ? lDims7.x : (x8+1)*2;
				
				// for (int y7 = y8*2; y7 < yEnd7; y7++) {
				int y7 = y8*2;
				for (int x7 = x8*2; x7 < xEnd7 && y7 < yEnd7; x7++) {
				if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x7, y7, sideID, 7)) {
				
					int yEnd6 = y7 == lDims7.y-1 ? lDims6.y : (y7+1)*2;
					int xEnd6 = x7 == lDims7.x-1 ? lDims6.x : (x7+1)*2;
					
					// for (int y6 = y7*2; y6 < yEnd6; y6++) {
					int y6 = y7*2;
					for (int x6 = x7*2; x6 < xEnd6 && y6 < yEnd6; x6++) {
					if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x6, y6, sideID, 6)) {
					
						int yEnd5 = y6 == lDims6.y-1 ? lDims5.y : (y6+1)*2;
						int xEnd5 = x6 == lDims6.x-1 ? lDims5.x : (x6+1)*2;
						
						// for (int y5 = y6*2; y5 < yEnd5; y5++) {
						int y5 = y6*2;
						for (int x5 = x6*2; x5 < xEnd5 && y5 < yEnd5; x5++) {
						if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x5, y5, sideID, 5)) {
						
							int yEnd4 = y5 == lDims5.y-1 ? lDims4.y : (y5+1)*2;
							int xEnd4 = x5 == lDims5.x-1 ? lDims4.x : (x5+1)*2;
							
							// for (int y4 = y5*2; y4 < yEnd4; y4++) {
							int y4 = y5*2;
							for (int x4 = x5*2; x4 < xEnd4 && y4 < yEnd4; x4++) {
							if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x4, y4, sideID, 4)) {
							
								int yEnd3 = y4 == lDims4.y-1 ? lDims3.y : (y4+1)*2;
								int xEnd3 = x4 == lDims4.x-1 ? lDims3.x : (x4+1)*2;
								
								// for (int y3 = y4*2; y3 < yEnd3; y3++) {
								int y3 = y4*2;
								for (int x3 = x4*2; x3 < xEnd3 && y3 < yEnd3; x3++) {
								if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x3, y3, sideID, 3)) {
								
									int yEnd2 = y3 == lDims3.y-1 ? lDims2.y : (y3+1)*2;
									int xEnd2 = x3 == lDims3.x-1 ? lDims2.x : (x3+1)*2;
									
									// for (int y2 = y3*2; y2 < yEnd2; y2++) {
									int y2 = y3*2;
									for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
									if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
									
										int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
										int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
										
										// for (int y1 = y2*2; y1 < yEnd1; y1++) {
										int y1 = y2*2;
										for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
										if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
										
											int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
											int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
											
											// for (int y0 = y1*2; y0 < yEnd0; y0++) {
											int y0 = y1*2;
											for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
											if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
											
												index[slowDim] = y0;
												index[fastDim] = x0;
												if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
													intersections++;
											
											}
											if (x0 == xEnd0-1) {
												x0 = x1*2-1;
												y0++;
											}
											}
										
										}
										if (x1 == xEnd1-1) {
											x1 = x2*2-1;
											y1++;
										}
										}
									
									}
									if (x2 == xEnd2-1) {
										x2 = x3*2-1;
										y2++;
									}
									}
								
								}
								if (x3 == xEnd3-1) {
									x3 = x4*2-1;
									y3++;
								}
								}
							
							}
							if (x4 == xEnd4-1) {
								x4 = x5*2-1;
								y4++;
							}
							}
						
						}
						if (x5 == xEnd5-1) {
							x5 = x6*2-1;
							y5++;
						}
						}
					
					}
					if (x6 == xEnd6-1) {
						x6 = x7*2-1;
						y6++;
					}
					}
				
				}
				if (x7 == xEnd7-1) {
					x7 = x8*2-1;
					y7++;
				}
				}
			
			}
			if (x8 == xEnd8-1) {
				x8 = x9*2-1;
				y8++;
			}
			}
		
		}
		if (x9 == xEnd9-1) {
			x9 = x10*2-1;
			y9++;
		}
		}
	
	}
	if (x10 == xEnd10-1) {
		x10 = x11*2-1;
		y10++;
	}
	}

	return intersections;
}

int SearchSideForInitialCellWithOctree_12Levels(vec3 origin, vec3 dir, float t0, int sideID, int fastDim, int slowDim, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
	ivec3 side = GetFaceFromFaceIndex(sideID);
	ivec3 index = (side+1)/2 * (cellDims-1);
	int intersections = 0;


	ivec2 lDims0 = GetBBoxArrayDimensions(sideID, 0);
	ivec2 lDims1 = GetBBoxArrayDimensions(sideID, 1);
	ivec2 lDims2 = GetBBoxArrayDimensions(sideID, 2);
	ivec2 lDims3 = GetBBoxArrayDimensions(sideID, 3);
	ivec2 lDims4 = GetBBoxArrayDimensions(sideID, 4);
	ivec2 lDims5 = GetBBoxArrayDimensions(sideID, 5);
	ivec2 lDims6 = GetBBoxArrayDimensions(sideID, 6);
	ivec2 lDims7 = GetBBoxArrayDimensions(sideID, 7);
	ivec2 lDims8 = GetBBoxArrayDimensions(sideID, 8);
	ivec2 lDims9 = GetBBoxArrayDimensions(sideID, 9);
	ivec2 lDims10 = GetBBoxArrayDimensions(sideID, 10);
	ivec2 lDims11 = GetBBoxArrayDimensions(sideID, 11);

	int yEnd11 = lDims11.y;
	int xEnd11 = lDims11.x;
	int x12 = 0;
	int y12 = 0;
	
	// for (int y11 = y12*2; y11 < yEnd11; y11++) {
	int y11 = y12*2;
	for (int x11 = x12*2; x11 < xEnd11 && y11 < yEnd11; x11++) {
	if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x11, y11, sideID, 11)) {
	
		int yEnd10 = y11 == lDims11.y-1 ? lDims10.y : (y11+1)*2;
		int xEnd10 = x11 == lDims11.x-1 ? lDims10.x : (x11+1)*2;
		
		// for (int y10 = y11*2; y10 < yEnd10; y10++) {
		int y10 = y11*2;
		for (int x10 = x11*2; x10 < xEnd10 && y10 < yEnd10; x10++) {
		if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x10, y10, sideID, 10)) {
		
			int yEnd9 = y10 == lDims10.y-1 ? lDims9.y : (y10+1)*2;
			int xEnd9 = x10 == lDims10.x-1 ? lDims9.x : (x10+1)*2;
			
			// for (int y9 = y10*2; y9 < yEnd9; y9++) {
			int y9 = y10*2;
			for (int x9 = x10*2; x9 < xEnd9 && y9 < yEnd9; x9++) {
			if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x9, y9, sideID, 9)) {
			
				int yEnd8 = y9 == lDims9.y-1 ? lDims8.y : (y9+1)*2;
				int xEnd8 = x9 == lDims9.x-1 ? lDims8.x : (x9+1)*2;
				
				// for (int y8 = y9*2; y8 < yEnd8; y8++) {
				int y8 = y9*2;
				for (int x8 = x9*2; x8 < xEnd8 && y8 < yEnd8; x8++) {
				if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x8, y8, sideID, 8)) {
				
					int yEnd7 = y8 == lDims8.y-1 ? lDims7.y : (y8+1)*2;
					int xEnd7 = x8 == lDims8.x-1 ? lDims7.x : (x8+1)*2;
					
					// for (int y7 = y8*2; y7 < yEnd7; y7++) {
					int y7 = y8*2;
					for (int x7 = x8*2; x7 < xEnd7 && y7 < yEnd7; x7++) {
					if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x7, y7, sideID, 7)) {
					
						int yEnd6 = y7 == lDims7.y-1 ? lDims6.y : (y7+1)*2;
						int xEnd6 = x7 == lDims7.x-1 ? lDims6.x : (x7+1)*2;
						
						// for (int y6 = y7*2; y6 < yEnd6; y6++) {
						int y6 = y7*2;
						for (int x6 = x7*2; x6 < xEnd6 && y6 < yEnd6; x6++) {
						if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x6, y6, sideID, 6)) {
						
							int yEnd5 = y6 == lDims6.y-1 ? lDims5.y : (y6+1)*2;
							int xEnd5 = x6 == lDims6.x-1 ? lDims5.x : (x6+1)*2;
							
							// for (int y5 = y6*2; y5 < yEnd5; y5++) {
							int y5 = y6*2;
							for (int x5 = x6*2; x5 < xEnd5 && y5 < yEnd5; x5++) {
							if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x5, y5, sideID, 5)) {
							
								int yEnd4 = y5 == lDims5.y-1 ? lDims4.y : (y5+1)*2;
								int xEnd4 = x5 == lDims5.x-1 ? lDims4.x : (x5+1)*2;
								
								// for (int y4 = y5*2; y4 < yEnd4; y4++) {
								int y4 = y5*2;
								for (int x4 = x5*2; x4 < xEnd4 && y4 < yEnd4; x4++) {
								if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x4, y4, sideID, 4)) {
								
									int yEnd3 = y4 == lDims4.y-1 ? lDims3.y : (y4+1)*2;
									int xEnd3 = x4 == lDims4.x-1 ? lDims3.x : (x4+1)*2;
									
									// for (int y3 = y4*2; y3 < yEnd3; y3++) {
									int y3 = y4*2;
									for (int x3 = x4*2; x3 < xEnd3 && y3 < yEnd3; x3++) {
									if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x3, y3, sideID, 3)) {
									
										int yEnd2 = y3 == lDims3.y-1 ? lDims2.y : (y3+1)*2;
										int xEnd2 = x3 == lDims3.x-1 ? lDims2.x : (x3+1)*2;
										
										// for (int y2 = y3*2; y2 < yEnd2; y2++) {
										int y2 = y3*2;
										for (int x2 = x3*2; x2 < xEnd2 && y2 < yEnd2; x2++) {
										if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x2, y2, sideID, 2)) {
										
											int yEnd1 = y2 == lDims2.y-1 ? lDims1.y : (y2+1)*2;
											int xEnd1 = x2 == lDims2.x-1 ? lDims1.x : (x2+1)*2;
											
											// for (int y1 = y2*2; y1 < yEnd1; y1++) {
											int y1 = y2*2;
											for (int x1 = x2*2; x1 < xEnd1 && y1 < yEnd1; x1++) {
											if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x1, y1, sideID, 1)) {
											
												int yEnd0 = y1 == lDims1.y-1 ? lDims0.y : (y1+1)*2;
												int xEnd0 = x1 == lDims1.x-1 ? lDims0.x : (x1+1)*2;
												
												// for (int y0 = y1*2; y0 < yEnd0; y0++) {
												int y0 = y1*2;
												for (int x0 = x1*2; x0 < xEnd0 && y0 < yEnd0; x0++) {
												if (IntersectRaySideCellBBoxDirect(origin, dir, t0, x0, y0, sideID, 0)) {
												
													index[slowDim] = y0;
													index[fastDim] = x0;
													if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
														intersections++;
												
												}
												if (x0 == xEnd0-1) {
													x0 = x1*2-1;
													y0++;
												}
												}
											
											}
											if (x1 == xEnd1-1) {
												x1 = x2*2-1;
												y1++;
											}
											}
										
										}
										if (x2 == xEnd2-1) {
											x2 = x3*2-1;
											y2++;
										}
										}
									
									}
									if (x3 == xEnd3-1) {
										x3 = x4*2-1;
										y3++;
									}
									}
								
								}
								if (x4 == xEnd4-1) {
									x4 = x5*2-1;
									y4++;
								}
								}
							
							}
							if (x5 == xEnd5-1) {
								x5 = x6*2-1;
								y5++;
							}
							}
						
						}
						if (x6 == xEnd6-1) {
							x6 = x7*2-1;
							y6++;
						}
						}
					
					}
					if (x7 == xEnd7-1) {
						x7 = x8*2-1;
						y7++;
					}
					}
				
				}
				if (x8 == xEnd8-1) {
					x8 = x9*2-1;
					y8++;
				}
				}
			
			}
			if (x9 == xEnd9-1) {
				x9 = x10*2-1;
				y9++;
			}
			}
		
		}
		if (x10 == xEnd10-1) {
			x10 = x11*2-1;
			y10++;
		}
		}
	
	}
	if (x11 == xEnd11-1) {
		x11 = x12*2-1;
		y11++;
	}
	}

	return intersections;
}

