#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <vapor/GridHelper.h>
using namespace Wasp;
using namespace VAPoR;



namespace {

// Format a vector as a space-separated element string
//
template <class T>
string vector_to_string(vector <T> v) {
	ostringstream oss;

	oss << "[";
	for (int i=0; i<v.size(); i++) {
		oss << v[i] << " ";
	}
	oss << "]";
	return(oss.str());
}


bool isUnstructured2D(
	const DC::Mesh &m,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <vector <string>> &cdimnames
) {
	DC::Mesh::Type mtype = m.GetMeshType();
	if (mtype == DC::Mesh::UNSTRUC_2D) {
		return (true);
	}
	return(false);
}

bool isUnstructuredLayered(
	const DC::Mesh &m,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <vector <string>> &cdimnames
) {
	DC::Mesh::Type mtype = m.GetMeshType();
	if (mtype == DC::Mesh::UNSTRUC_LAYERED) {
		return (true);
	}
	return(false);
}

bool isRegular(
	const DC::Mesh &m,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <vector <string>> &cdimnames
) {
	VAssert(cvarsinfo.size() == cdimnames.size());

	for (int i=0; i<cdimnames.size(); i++) {
		if (! (cdimnames[i].size() == 1 && cvarsinfo[i].GetUniform())) {
			return(false);
		}
	}
	return(true);
}


bool isStretched(
	const DC::Mesh &m,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <vector <string>> &cdimnames
) {
	VAssert(cvarsinfo.size() == cdimnames.size());

	// All dimensions need to be 1D and at least one non-uniform
	//
	for (int i=0; i<cdimnames.size(); i++) {
		if (! (cdimnames[i].size() == 1)) { 
			return(false);
		}
	}

	// Need at least one non-uniform dimension
	//
	for (int i=0; i<cvarsinfo.size(); i++) {
		if (! (cvarsinfo[i].GetUniform())) { 
			return(true);
		}
	}

	return(false);
}

bool isLayered(
	const DC::Mesh &m,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <vector <string>> &cdimnames
) {
	VAssert(cvarsinfo.size() == cdimnames.size());

	if (cdimnames.size() != 3) return(false);

	if (! (cvarsinfo[0].GetUniform() && cvarsinfo[1].GetUniform())) {
		return(false);
	} 

	if (! (cdimnames[2].size() == 3)) return(false);

	return(true);
}

bool isCurvilinear(
	const DC::Mesh &m,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <vector <string>> &cdimnames
) {

	VAssert(cvarsinfo.size() == cdimnames.size());

	if (! (cdimnames.size() == 2 || cdimnames.size() == 3)) return(false);

	if (cdimnames.size() == 3 && ! 
		((cdimnames[2].size() == 1) || (cdimnames[2].size() == 3))) {

		return(false);
	}

	if (! (cdimnames[0].size() == 2 && cdimnames[1].size() == 2)) return(false);

	return(true);
}

};

using namespace VAPoR;
using namespace Wasp;


const KDTreeRG *GridHelper::_getKDTree2D(
	size_t ts,
	int level,
	int lod,
    const vector <DC::CoordVar> &cvarsinfo, 
	const Grid &xg,
	const Grid &yg,
	const vector <size_t> &bmin,
	const vector <size_t> &bmax
) {
	VAssert(cvarsinfo.size() >= 2);
	VAssert(xg.GetDimensions() == yg.GetDimensions());

	vector <string> varnames;
	for (int i=0; i<cvarsinfo.size(); i++) {
		varnames.push_back(cvarsinfo[i].GetName());
	}

	bool time_varying = false;
	for (int i=0; i<cvarsinfo.size(); i++) {
		if (! cvarsinfo[i].GetTimeDimName().empty()) {
			time_varying = true;
		}
	}

	if (! time_varying) {
		ts = 0;
	}

    ostringstream oss;

    oss << ts;
	oss << ":";
	oss << vector_to_string(varnames);
	oss << ":";
	oss << level;
	oss << ":";
	oss << vector_to_string(bmin);
	oss << ":";
	oss << vector_to_string(bmax);

	string key = oss.str();

	KDTreeRG *kdtree = _kdtreeCache.get(key);
	if (kdtree) {
		return(kdtree);
	}

	kdtree = new KDTreeRG(xg, yg);
	
	KDTreeRG *oldkdtree = _kdtreeCache.put(key, kdtree);
	if (oldkdtree) {
		delete oldkdtree;
	}
	return(kdtree);
}

RegularGrid *GridHelper::_make_grid_regular(
	const vector <size_t> &dims,
    const vector <float *> &blkvec,
	const vector <size_t> &bs,
	const vector <size_t> &bmin,
	const vector <size_t> &bmax

) const {
	VAssert (dims.size() == bs.size());
	VAssert (dims.size() == bmin.size());
	VAssert (dims.size() == bmax.size());

	vector <double> minu, maxu;
	for (int i = 0; i < dims.size(); i++) { 
		float *coords = blkvec[i+1];
		minu.push_back(coords[0]);
		maxu.push_back(coords[dims[i]-1]);
	}


	size_t nblocks = 1;
	size_t block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }

	vector <float *> blkptrs;
	if (blkvec[0]) {
		for (int i=0; i<nblocks; i++) {
			blkptrs.push_back(blkvec[0] + i*block_size);
		}
	}

	RegularGrid *rg = new RegularGrid(dims, bs, blkptrs, minu, maxu);

	return(rg);
}

StretchedGrid *GridHelper::_make_grid_stretched(
	const vector <size_t> &dims,
    const vector <float *> &blkvec,
	const vector <size_t> &bs,
	const vector <size_t> &bmin,
	const vector <size_t> &bmax

) const {
	VAssert (dims.size() == bs.size());
	VAssert (dims.size() == bmin.size());
	VAssert (dims.size() == bmax.size());

	size_t nblocks = 1;
	size_t block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }

	vector <float *> blkptrs;
	if (blkvec[0]) {
		for (int i=0; i<nblocks; i++) {
			blkptrs.push_back(blkvec[0] + i*block_size);
		}
	}

	vector <double> xcoords;
	for (int i=0; i<dims[0]; i++) xcoords.push_back(blkvec[1][i]);

	vector <double> ycoords;
	for (int i=0; i<dims[1]; i++) ycoords.push_back(blkvec[2][i]);

	vector <double> zcoords;
	if (dims.size()==3) {
		for (int i=0; i<dims[2]; i++) zcoords.push_back(blkvec[3][i]);
	}

	StretchedGrid *sg = new StretchedGrid(
		dims, bs, blkptrs, xcoords, ycoords, zcoords
	);

	return(sg);
}

LayeredGrid *GridHelper::_make_grid_layered(
	const vector <size_t> &dims,
    const vector <float *> &blkvec,
	const vector <size_t > &bs,
	const vector <size_t > &bmin,
	const vector <size_t > &bmax
) const {
	VAssert(bs.size() == bmin.size());
	VAssert(bs.size() == bmax.size());
	VAssert (dims.size() == bs.size());
	VAssert (dims.size() == bmin.size());
	VAssert (dims.size() == bmax.size());

	// Get horizontal dimensions
	//
	vector <double> hminu, hmaxu;
	for (int i = 0; i<2; i++) { 
		float *coords = blkvec[i+1];
		hminu.push_back(coords[0]);
		hmaxu.push_back(coords[dims[i]-1]);
	}

	// Data blocks
	//
	size_t nblocks = 1;
	size_t block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }

	vector <float *> blkptrs, zcblkptrs;

	if (blkvec[0]) {
		for (int i=0; i<nblocks; i++) {
			blkptrs.push_back(blkvec[0] + i*block_size);
		}
	}

	// Z Coord blocks
	//
	nblocks = 1;
	block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }
    for (int i=0; i<nblocks; i++) {
        zcblkptrs.push_back(blkvec[3] + i*block_size);
	}


	RegularGrid rg(
		dims, bs, zcblkptrs, vector <double> (3,0.0), vector <double> (3,1.0)
	);

	LayeredGrid *lg = new LayeredGrid(dims, bs, blkptrs, hminu, hmaxu, rg);

	return(lg);
}

CurvilinearGrid *GridHelper::_make_grid_curvilinear(
	size_t ts,
	int level,
	int lod,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <size_t> &dims,
    const vector <float *> &blkvec,
	const vector <size_t> &bs,
	const vector <size_t> &bmin,
	const vector <size_t> &bmax
) {
	VAssert(bs.size() == bmin.size());
	VAssert(bs.size() == bmax.size());
	VAssert (dims.size() == bs.size());
	VAssert (dims.size() == bmin.size());
	VAssert (dims.size() == bmax.size());

	// Data blocks
	//
	size_t nblocks = 1;
	size_t block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }

	// block pointers for data
	//
	vector <float *> blkptrs;
    for (int i=0; i<nblocks; i++) {
        if (blkvec[0]) blkptrs.push_back(blkvec[0] + i*block_size);
	}

	// X horizontal coord blocks
	//
	vector <size_t> bs2d = {bs[0], bs[1]};
	size_t nblocks2d = 1;
	size_t block_size2d = 1;
    for (int i=0; i<bs2d.size(); i++) {
        nblocks2d *= bmax[i]-bmin[i]+1;
        block_size2d *= bs2d[i];
    }

	vector <float *> xcblkptrs;
    for (int i=0; i<nblocks2d; i++) {
        xcblkptrs.push_back(blkvec[1] + i*block_size2d);
	}

	// Y horizontal coord blocks
	//
	nblocks2d = 1;
	block_size2d = 1;
    for (int i=0; i<bs2d.size(); i++) {
        nblocks2d *= bmax[i]-bmin[i]+1;
        block_size2d *= bs2d[i];
    }
	vector <float *> ycblkptrs;
    for (int i=0; i<nblocks2d; i++) {
        ycblkptrs.push_back(blkvec[2] + i*block_size2d);
	}

	vector <double> minu2d = {0.0, 0.0};
	vector <double> maxu2d = {1.0, 1.0};
	vector <size_t> dims2d = {dims[0], dims[1]};
	RegularGrid xrg(dims2d, bs2d, xcblkptrs, minu2d, maxu2d);
	RegularGrid yrg(dims2d, bs2d, ycblkptrs, minu2d, maxu2d);

	const KDTreeRG *kdtree = _getKDTree2D(
		ts, level, lod, cvarsinfo, xrg, yrg, bmin, bmax
	);

	if (dims.size() == 3 && cvarsinfo[2].GetDimNames().size() == 3) {

		// Terrain following vertical
		//
		vector <double> minu = {0.0, 0.0, 0.0};
		vector <double> maxu = {1.0, 1.0, 1.0};

		vector <float *> zcblkptrs;
		for (int i=0; i<nblocks; i++) {
			zcblkptrs.push_back(blkvec[3] + i*block_size);
		}

		RegularGrid zrg(dims, bs, zcblkptrs, minu, maxu);

		return (new CurvilinearGrid(
			dims, bs, blkptrs, xrg, yrg, zrg, kdtree
		));

	}
	else if (dims.size() == 3 && cvarsinfo[2].GetDimNames().size() == 1) {

		// stretched vertical
		//
		vector <double> zcoords;
		for (int i=0; i<dims[2]; i++) zcoords.push_back(blkvec[3][i]);

		return (new CurvilinearGrid(
			dims, bs, blkptrs, xrg, yrg, zcoords, kdtree
		));
	}
	else {

		// 2D
		//
		return (new CurvilinearGrid(
			dims, bs, blkptrs, xrg, yrg, vector <double> (), kdtree
		));

	}
}



UnstructuredGrid2D *GridHelper::_make_grid_unstructured2d(
	size_t ts,
	int level,
	int lod,
	const DC::DataVar &var,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <size_t> &dims,
    const vector <float *> &blkvec,
	const vector <size_t> &bs,
	const vector <size_t> &bmin,
	const vector <size_t> &bmax,
    const vector <int *> &conn_blkvec,
	const vector <size_t> &conn_bs,
	const vector <size_t> &conn_bmin,
	const vector <size_t> &conn_bmax,
	const vector <size_t> &vertexDims,
	const vector <size_t> &faceDims,
	const vector <size_t> &edgeDims,
    UnstructuredGrid::Location location,
    size_t maxVertexPerFace,
    size_t maxFacePerVertex,
	long vertexOffset, 
	long faceOffset
) {
	VAssert (dims.size() == 1);
	VAssert (dims.size() == bs.size());
	VAssert (dims.size() == bmin.size());
	VAssert (dims.size() == bmax.size());
	VAssert(blkvec.size() == 3);

	VAssert(conn_blkvec.size() >= 2);
	

	// block pointers for data
	//
	size_t nblocks = 1;
	size_t block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }

	vector <float *> blkptrs;
    for (int i=0; i<nblocks; i++) {
        if (blkvec[0]) blkptrs.push_back(blkvec[0] + i*block_size);
	}


	// Block pointers for X coordinates, which are always 1D
	//
	nblocks = 1;
	block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }

	vector <float *> xcblkptrs;
    for (int i=0; i<nblocks; i++) {
        xcblkptrs.push_back(blkvec[1] + i*block_size);
	}

	// Block pointers for X coordinates, which are always 1D
	//
	nblocks = 1;
	block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }
	vector <float *> ycblkptrs;
    for (int i=0; i<nblocks; i++) {
        ycblkptrs.push_back(blkvec[2] + i*block_size);
	}

	// N.B. assumes blkvec contains contiguous blocks :-(
	//
	const int *vertexOnFace = conn_blkvec[0];
	const int *faceOnVertex = conn_blkvec[1];
	const int *faceOnFace = conn_blkvec.size()==3 ? conn_blkvec[2] : NULL;

	UnstructuredGridCoordless xug(
		vertexDims, faceDims, edgeDims, bs, xcblkptrs, 2,
		vertexOnFace, faceOnVertex, faceOnFace, location,
		maxVertexPerFace, maxFacePerVertex
	);
	xug.SetNodeOffset(vertexOffset);
	xug.SetCellOffset(faceOffset);

	UnstructuredGridCoordless yug(
		vertexDims, faceDims, edgeDims, bs, ycblkptrs, 2,
		vertexOnFace, faceOnVertex, faceOnFace, location,
		maxVertexPerFace, maxFacePerVertex
	);
	yug.SetNodeOffset(vertexOffset);
	yug.SetCellOffset(faceOffset);

	UnstructuredGridCoordless zug;

	const KDTreeRG *kdtree = _getKDTree2D(
		ts, level, lod, cvarsinfo, xug, yug, bmin, bmax
	);

	UnstructuredGrid2D *g = new UnstructuredGrid2D(
		vertexDims, faceDims, edgeDims, bs, blkptrs, 
		vertexOnFace, faceOnVertex, faceOnFace, location,
		maxVertexPerFace, maxFacePerVertex,
		xug, yug, zug, kdtree
	);
	g->SetNodeOffset(vertexOffset);
	g->SetCellOffset(faceOffset);

	return(g);
}

UnstructuredGridLayered *GridHelper::_make_grid_unstructured_layered(
	size_t ts,
	int level,
	int lod,
	const DC::DataVar &var,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <size_t> &dims,
    const vector <float *> &blkvec,
	const vector <size_t> &bs,
	const vector <size_t> &bmin,
	const vector <size_t> &bmax,
    const vector <int *> &conn_blkvec,
	const vector <size_t> &conn_bs,
	const vector <size_t> &conn_bmin,
	const vector <size_t> &conn_bmax,
	const vector <size_t> &vertexDims,
	const vector <size_t> &faceDims,
	const vector <size_t> &edgeDims,
    UnstructuredGrid::Location location,
    size_t maxVertexPerFace,
    size_t maxFacePerVertex,
	long vertexOffset, 
	long faceOffset
) {
	VAssert (dims.size() == 2);
	VAssert (dims.size() == bs.size());
	VAssert (dims.size() == bmin.size());
	VAssert (dims.size() == bmax.size());
	VAssert(blkvec.size() == 4);

	VAssert(conn_blkvec.size() >= 2);
	

	// block pointers for data
	//
	size_t nblocks = 1;
	size_t block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }

	vector <float *> blkptrs;
    for (int i=0; i<nblocks; i++) {
        if (blkvec[0]) blkptrs.push_back(blkvec[0] + i*block_size);
	}


	// Block pointers for X coordinates, which are always 1D
	//
	nblocks = 1;
	block_size = 1;
	vector <size_t> bs1d = {bs[0]};
    for (int i=0; i<bs1d.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs1d[i];
    }

	vector <float *> xcblkptrs;
    for (int i=0; i<nblocks; i++) {
        xcblkptrs.push_back(blkvec[1] + i*block_size);
	}

	// Block pointers for Y coordinates, which are always 1D
	//
	nblocks = 1;
	block_size = 1;
    for (int i=0; i<bs1d.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs1d[i];
    }
	vector <float *> ycblkptrs;
    for (int i=0; i<nblocks; i++) {
        ycblkptrs.push_back(blkvec[2] + i*block_size);
	}

	// Block pointers for Z coordinates, which are always 2D
	//
	nblocks = 1;
	block_size = 1;
    for (int i=0; i<bs.size(); i++) {
        nblocks *= bmax[i]-bmin[i]+1;
        block_size *= bs[i];
    }
	vector <float *> zcblkptrs;
    for (int i=0; i<nblocks; i++) {
        zcblkptrs.push_back(blkvec[3] + i*block_size);
	}

	// N.B. assumes blkvec contains contiguous blocks :-(
	//
	const int *vertexOnFace = conn_blkvec[0];
	const int *faceOnVertex = conn_blkvec[1];
	const int *faceOnFace = conn_blkvec.size()==3 ? conn_blkvec[2] : NULL;

	vector <size_t> vertexDims1D = {vertexDims[0]};
	vector <size_t> faceDims1D = {faceDims[0]};
	vector <size_t> edgeDims1D; 
	if (edgeDims.size()) {
		edgeDims1D.push_back(edgeDims[0]);
	}

	UnstructuredGridCoordless xug(
		vertexDims1D, faceDims1D, edgeDims1D, bs1d, xcblkptrs, 2,
		vertexOnFace, faceOnVertex, faceOnFace, location,
		maxVertexPerFace, maxFacePerVertex
	);
	xug.SetNodeOffset(vertexOffset);
	xug.SetCellOffset(faceOffset);

	UnstructuredGridCoordless yug(
		vertexDims1D, faceDims1D, edgeDims1D, bs1d, ycblkptrs, 2,
		vertexOnFace, faceOnVertex, faceOnFace, location,
		maxVertexPerFace, maxFacePerVertex
	);
	yug.SetNodeOffset(vertexOffset);
	yug.SetCellOffset(faceOffset);

	UnstructuredGridCoordless zug(
		vertexDims, faceDims, edgeDims, bs, zcblkptrs, 3,
		vertexOnFace, faceOnVertex, faceOnFace, location,
		maxVertexPerFace, maxFacePerVertex
	);
	zug.SetNodeOffset(vertexOffset);
	zug.SetCellOffset(faceOffset);

	const KDTreeRG *kdtree = _getKDTree2D(
		ts, level, lod, cvarsinfo, xug, yug, bmin, bmax
	);

	UnstructuredGridLayered *g = new UnstructuredGridLayered(
		vertexDims, faceDims, edgeDims, bs, blkptrs, 
		vertexOnFace, faceOnVertex, faceOnFace, location,
		maxVertexPerFace, maxFacePerVertex,
		xug, yug, zug, kdtree
	);
	g->SetNodeOffset(vertexOffset);
	g->SetCellOffset(faceOffset);

	return(g);
}


void GridHelper::_makeGridHelper(
    const DC::DataVar &var,
    const vector <size_t> &roi_dims,
    const vector <size_t> &dims,
	Grid *g
) const {
	VAssert(roi_dims.size() == dims.size());

	vector <bool> has_periodic = var.GetPeriodic();
    vector <bool> periodic;
	for (int i=0; i<dims.size(); i++) {
		if (i<has_periodic.size() && has_periodic[i] && roi_dims[i]==dims[i]) {
			periodic.push_back(true);
		}
		else {
			periodic.push_back(false);
		}
	}


	bool has_missing = false;
	float mv = 0.0;
	if (var.GetHasMissing()) {
		has_missing = true;
		mv = var.GetMissingValue();
	}

	g->SetPeriodic(periodic);
	if (has_missing) {
		g->SetHasMissingValues(true);
		g->SetMissingValue(mv);
	}

}

//	var: variable info
//  roi_dims: spatial dimensions of ROI
//	dims: spatial dimensions of full variable domain in voxels
//	blkvec: data blocks, and coordinate blocks
//	bsvec: data block dimensions, and coordinate block dimensions
//  bminvec: ROI offsets in blocks, full domain, data and coordinates
//  bmaxvec: ROI offsets in blocks, full domain, data and coordinates
//

StructuredGrid *GridHelper::MakeGridStructured(
	string gridType,
	size_t ts,
	int level,
	int lod,
	const DC::DataVar &var,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <size_t> &roi_dims, 
	const vector <size_t> &dims,
	const vector <float *> &blkvec,
	const vector < vector <size_t > > &bsvec,
	const vector < vector <size_t > > &bminvec,
	const vector < vector <size_t > > &bmaxvec
) {


	StructuredGrid *rg = NULL;
    if (gridType == RegularGrid::GetClassType()) {
		rg = _make_grid_regular(
			roi_dims, blkvec, bsvec[0], bminvec[0], bmaxvec[0]
		);
	}
    else if (gridType == StretchedGrid::GetClassType()) {
		rg = _make_grid_stretched(
			roi_dims, blkvec, bsvec[0], bminvec[0], bmaxvec[0]
		);
	}
    else if (gridType == LayeredGrid::GetClassType()) {
		rg = _make_grid_layered(
			roi_dims, blkvec, bsvec[0], bminvec[0], bmaxvec[0]
		);
	}
    else if (gridType == CurvilinearGrid::GetClassType()) {
		rg = _make_grid_curvilinear(
			ts, level, lod, cvarsinfo, roi_dims, 
			blkvec, bsvec[0], bminvec[0], bmaxvec[0]
		);
	}
	else {
		return(NULL);
	}

	
	_makeGridHelper(var, roi_dims, dims, rg);

	return(rg);
}

UnstructuredGrid *GridHelper::MakeGridUnstructured(
	string gridType,
	size_t ts,
	int level,
	int lod,
	const DC::DataVar &var,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <size_t> &roi_dims, 
	const vector <size_t> &dims,
	const vector <float *> &blkvec,
	const vector < vector <size_t > > &bsvec,
	const vector < vector <size_t > > &bminvec,
	const vector < vector <size_t > > &bmaxvec,
	const vector <int *> &conn_blkvec,
	const vector < vector <size_t > > &conn_bsvec,
	const vector < vector <size_t > > &conn_bminvec,
	const vector < vector <size_t > > &conn_bmaxvec,
	const vector <size_t> &vertexDims,
	const vector <size_t> &faceDims,
	const vector <size_t> &edgeDims,
    UnstructuredGrid::Location location,
    size_t maxVertexPerFace,
    size_t maxFacePerVertex,
	long vertexOffset, 
	long faceOffset
) {


	UnstructuredGrid *rg = NULL;

    if (gridType == UnstructuredGrid2D::GetClassType()) {
		rg = _make_grid_unstructured2d(
			ts, level, lod, var, cvarsinfo, roi_dims, 
			blkvec, bsvec[0], bminvec[0], bmaxvec[0],
			conn_blkvec, conn_bsvec[0], conn_bminvec[0], conn_bmaxvec[0],
			vertexDims, faceDims, edgeDims, location, maxVertexPerFace,
			maxFacePerVertex, vertexOffset, faceOffset
		);
	}
	else if (gridType == UnstructuredGridLayered::GetClassType()) {
		rg = _make_grid_unstructured_layered(
			ts, level, lod, var, cvarsinfo, roi_dims, 
			blkvec, bsvec[0], bminvec[0], bmaxvec[0],
			conn_blkvec, conn_bsvec[0], conn_bminvec[0], conn_bmaxvec[0],
			vertexDims, faceDims, edgeDims, location, maxVertexPerFace,
			maxFacePerVertex, vertexOffset, faceOffset
		);
	}
	else {
		return(NULL);
	}

	_makeGridHelper(var, roi_dims, dims, rg);

	return(rg);
}



GridHelper::~GridHelper() {
	KDTreeRG *kdtree;

	while ((kdtree = _kdtreeCache.remove_lru()) != NULL) {
		delete kdtree;
	}
}


string GridHelper::GetGridType(
	const DC::Mesh &m,
	const vector <DC::CoordVar> &cvarsinfo,
	const vector <vector <string>> &cdimnames
) const {

	
	if (isUnstructured2D(m, cvarsinfo, cdimnames)) {
		return (UnstructuredGrid2D::GetClassType());
	}

	if (isUnstructuredLayered(m, cvarsinfo, cdimnames)) {
		return (UnstructuredGridLayered::GetClassType());
	}

	if (isRegular(m, cvarsinfo, cdimnames)) {
		return (RegularGrid::GetClassType());
	}

	if (isStretched(m, cvarsinfo, cdimnames)) {
		return (StretchedGrid::GetClassType());
	}

	if (isLayered(m, cvarsinfo, cdimnames)) {
		return (LayeredGrid::GetClassType());
	}

	if (isCurvilinear(m, cvarsinfo, cdimnames)) {
		return (CurvilinearGrid::GetClassType());
	}

	return("");
}

bool GridHelper::IsUnstructured(std::string gridType) const {

	return (
		gridType == UnstructuredGrid2D::GetClassType() ||
		gridType == UnstructuredGridLayered::GetClassType()
	); 

}

bool GridHelper::IsStructured(std::string gridType) const {

	return (
		(gridType == RegularGrid::GetClassType()) || 
		(gridType == StretchedGrid::GetClassType()) || 
		(gridType == LayeredGrid::GetClassType()) || 
		(gridType == CurvilinearGrid::GetClassType()) 
	);
}

