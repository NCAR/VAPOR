//************************************************************************

//		     Copyright (C)  2008										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//																		*
//************************************************************************/
//
//	File:		TwoDImagerRenderer.cpp
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		March 2016
//
//	Description:	Implementation of the twoDImageRenderer class
//

#include <vapor/glutil.h>	// Must be included first!!!

#include <iostream>
#include <fstream>

#include <vapor/Proj4API.h>
#include <vapor/CFuncs.h>
#include <vapor/AnimationParams.h>
#include <vapor/GeoImageGeoTiff.h>
#include <vapor/GeoImageTMS.h>
#include <vapor/TwoDImageRenderer.h>
#include <vapor/TwoDImageParams.h>

using namespace VAPoR;

namespace {


// Make mesh conformant. PCS coordinates may wrap around globe at 
// boundaries :-(
//
void conform(
	GLfloat *verts, int nx, int ny
) {
 	assert(nx>=2);
 	assert(ny>=2);

	// x values
	//
	for (int j=0; j<ny; j++) {

		// left side
		//
		if (verts[3*(nx*j)] > verts[3*(nx*j+1)]) {
			verts[3*(nx*j)] = verts[3*(nx*j+1)];
		}

		// right side
		//
		if (verts[3*((nx*j)+(nx-1))] < verts[3*((nx*j)+(nx-2))]) {
			verts[3*((nx*j)+(nx-1))] = verts[3*((nx*j)+(nx-2))];
		}
	}

	// y values
	//
	verts++;	
	for (int i=0; i<nx; i++) {
		// bottom row
		//
		if (verts[3*i] > verts[3*(nx+i)]) {
			verts[3*i] = verts[3*(nx+i)];
		}

		// top row
		//
		if (verts[3*(nx*(ny-1)+i)] < verts[3*(nx*(ny-2)+i)]) {
			verts[3*(nx*(ny-1)+i)] = verts[3*(nx*(ny-2)+i)];
		}
	}
}

};

TwoDImageRenderer::TwoDImageRenderer(
	Visualizer* v, RenderParams* rp, ShaderMgr *sm
) : TwoDRenderer(v, rp, sm) {

	_geoImage = NULL;
	_cacheImgFileName.clear();
	_twoDTex = NULL;
	_cacheTimes.clear();
	_pcsExtentsData.clear();

	for (int i=0; i<4; i++) {
		_pcsExtentsImg[i] = 0;
	}
    _proj4StringImg.clear();
    _texWidth = 0;
    _texHeight = 0;
	_cacheTimestep = 0;
	_cacheRefLevel = 0;
	_cacheLod = 0;
	_cacheHgtVar = "";
	_cacheGeoreferenced = -1;
	_cacheTimestepTex = 0;
	_cacheBoxExtentsTex.clear();
	_vertsWidth = 0;
	_vertsHeight = 0;
}

TwoDImageRenderer::~TwoDImageRenderer()
{
	if (_geoImage) delete _geoImage;
	_geoImage = NULL;
}

const GLvoid *TwoDImageRenderer::_GetTexture(
	DataMgr *dataMgr,
	GLsizei &width,
	GLsizei &height,
	GLint &internalFormat,
	GLenum &format,
	GLenum &type,
	size_t &texelSize
) {
	width = 0;
	height = 0;
	internalFormat = GL_RGBA;
	format = GL_RGBA;
	type = GL_UNSIGNED_BYTE;
	texelSize = 4;	// RGBA * sizeof(type)

	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();

	if (myParams->GetIgnoreTransparency()) internalFormat = GL_RGB;

	GLvoid *texture = (GLvoid *) _getTexture(dataMgr);
	if (! texture) return(NULL);

	width = _texWidth;
	height = _texHeight;
	return(texture);
}
	
	
int TwoDImageRenderer::_GetMesh(
	DataMgr *dataMgr,
	GLfloat **verts,
	GLfloat **normals,
	GLsizei &width,
	GLsizei &height
) {
	width = 0;
	height = 0;

	// See if already in cache
	//
	if (! _gridStateDirty() && _sb_verts.GetBuf()) {
		width = _vertsWidth;
		height = _vertsHeight;
		*verts = (GLfloat *) _sb_verts.GetBuf();
		*normals = (GLfloat *) _sb_normals.GetBuf();
		return(0);
	}
	_gridStateClear();

	AnimationParams* myAnimationParams = _visualizer->getActiveAnimationParams();
	size_t ts = myAnimationParams->GetCurrentTimestep();

	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();

    // Find box extents for ROI
	//
    vector<double> minBoxReq, maxBoxReq;
    myParams->GetBox()->GetUserExtents(minBoxReq, maxBoxReq, ts);

	// Get scene scaling factors
	//
	VizFeatureParams *vizfeature =_visualizer->getActiveVizFeatureParams();
	vector <double> stretchFac = vizfeature->GetStretchFactors();
	assert(stretchFac.size() == 3);

	int rc;

	// If we are terrain mapping the image or if both the image and the
	// data are geo-referenced
	//
	if (! myParams->GetHeightVariableName().empty() || 
		(myParams->GetGeoreferenced() && !dataMgr->GetMapProjection().empty())){

		// Get the width and height of the image texture. These
		// will be used to set the width and height of the mesh.
		//
		unsigned char *texture = (unsigned char *) _getTexture(dataMgr);

		_vertsWidth = _texWidth;
		_vertsHeight = _texHeight;
		rc = _getMeshDisplaced(
			dataMgr, _vertsWidth, _vertsHeight, minBoxReq, maxBoxReq
		);
	}
	else {
		_vertsWidth = 2;
		_vertsHeight = 2;
		rc = _getMeshPlane(minBoxReq, maxBoxReq);
	}
	
	if (rc < 0) {
		_vertsWidth = 0;
		_vertsHeight = 0;
		return(-1);
	}

	// Ugh. Rendering is done in "local", not absolute coordinates.
	// Transform to local, and apply scaling factors
	//
	_transformToLocal(_vertsWidth, _vertsHeight, stretchFac);


	_gridStateSet();


	// Compute vertex normals
	//
	*verts = (GLfloat *) _sb_verts.GetBuf();
	*normals = (GLfloat *) _sb_normals.GetBuf();
	_ComputeNormals(*verts, _vertsWidth, _vertsHeight, *normals);

	width = _vertsWidth;
	height = _vertsHeight;

	return(0);
}


// Sets _pcsExtentsImg, _pcsExtentsData, 
// _proj4StringImg, _texWidth, _texHeight
//
unsigned char *TwoDImageRenderer::_getTexture(
	DataMgr* dataMgr
) {
	
	AnimationParams* myAnimationParams = _visualizer->getActiveAnimationParams();
	TwoDImageParams* myParams = (TwoDImageParams*) getRenderParams();
	
	int currentTimestep =  myAnimationParams->GetCurrentTimestep();
	
	string imgFileName = myParams->GetImageFileName();
	vector <double> times = dataMgr->GetTimeCoordinates();

	// Initialize _geoImage if image file or user times have changed
	//
	if (_imageStateDirty(times)) {
		int rc = _reinit(imgFileName, times);
		if(rc<0) return(NULL);
	}
	assert(_geoImage);

	// Get the ROI for the displayed image in PCS coordinates of 
	// the data
	//
	vector <double> _pcsExtentsData = _getPCSExtentsData();

	// Get a new texture if any relevant parameters have changed
	//
	if (!_twoDTex || _imageStateDirty(times) || _texStateDirty(dataMgr)) {

		// Get pro4 string for data if georeferencing is requested
		//
		string proj4StringData;
		if (myParams->GetGeoreferenced()) {
			proj4StringData = dataMgr->GetMapProjection();
		}

		double geoCornersImg[8];  // Coordinates of image corners in geo coords

		_twoDTex = _getImage(
			_geoImage, currentTimestep, proj4StringData, _pcsExtentsData, 
			_pcsExtentsImg, geoCornersImg, _proj4StringImg,
			_texWidth, _texHeight
		);

		if (! _twoDTex) return(NULL);

		_texStateSet(dataMgr);

		// Force recompute of mesh
		//
		_gridStateClear();

	}
	_imageStateSet(times);

	return(_twoDTex);
}

  
bool TwoDImageRenderer::_gridStateDirty() const {

	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();
	AnimationParams* myAnimationParams = _visualizer->getActiveAnimationParams();

	int refLevel = myParams->GetRefinementLevel();
	int lod = myParams->GetCompressionLevel();
	string hgtVar = myParams->GetHeightVariableName();
	int ts  = myAnimationParams->GetCurrentTimestep();
	vector <double> boxExtents = myParams->GetBox()->GetLocalExtents();

	return(
		refLevel != _cacheRefLevel ||
		lod != _cacheLod ||
		hgtVar != _cacheHgtVar ||
		ts != _cacheTimestep ||
		boxExtents != _cacheBoxExtents 
	);
}

void TwoDImageRenderer::_gridStateClear() {
	_cacheRefLevel = 0;
	_cacheLod  = 0;
	_cacheHgtVar.clear();
	_cacheTimestep  = -1;
	_cacheBoxExtents.clear();
}

void TwoDImageRenderer::_gridStateSet(
) {
	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();
	AnimationParams* myAnimationParams = _visualizer->getActiveAnimationParams();
	_cacheRefLevel = myParams->GetRefinementLevel();
	_cacheLod = myParams->GetCompressionLevel();
	_cacheHgtVar = myParams->GetHeightVariableName();
	_cacheTimestep = myAnimationParams->GetCurrentTimestep();
	_cacheBoxExtents = myParams->GetBox()->GetLocalExtents();
}

bool TwoDImageRenderer::_imageStateDirty(
	const vector <double> &times
) const {

	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();
	string imgFileName = myParams->GetImageFileName();

	return(
		_cacheImgFileName != imgFileName ||
		_cacheTimes != times 
	);
}

void TwoDImageRenderer::_imageStateSet(
	const vector <double> &times
) {

	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();

	string imgFileName = myParams->GetImageFileName();

	_cacheImgFileName = imgFileName;
	_cacheTimes = times;
}

void TwoDImageRenderer::_imageStateClear() {
	_cacheImgFileName.clear();
	_cacheTimes.clear();
}

bool TwoDImageRenderer::_texStateDirty(
	DataMgr *dataMgr
) const {

	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();
	AnimationParams* myAnimationParams = _visualizer->getActiveAnimationParams();

	int georeferenced = (int) myParams->GetGeoreferenced();
	int ts  = myAnimationParams->GetCurrentTimestep();
	vector <double> boxExtents = myParams->GetBox()->GetLocalExtents();

	return(
		_cacheTimestepTex != ts ||
		_cacheBoxExtentsTex != boxExtents ||
		_cacheGeoreferenced != georeferenced
	);
}

void TwoDImageRenderer::_texStateSet(
	DataMgr *dataMgr
) {

	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();
	AnimationParams* myAnimationParams = _visualizer->getActiveAnimationParams();
	int georeferenced = (int) myParams->GetGeoreferenced();

	_cacheTimestepTex = myAnimationParams->GetCurrentTimestep();
	_cacheBoxExtentsTex = myParams->GetBox()->GetLocalExtents();
	_cacheGeoreferenced = georeferenced;
}

void TwoDImageRenderer::_texStateClear() {
	_cacheTimestepTex = -1;
	_cacheBoxExtentsTex.clear();
	_cacheGeoreferenced = -1;
}


int TwoDImageRenderer::_reinit(
	string path,
	vector <double> times
) {

	// Two forms of georeferenced images are cachely supported. 
	// If path is a directory then we have a TMS database. Otherwise, 
	// path must point to a tiff file
	//

	bool tms_flag = false;
	if (path.rfind(".tms", path.size()-4) != string::npos) {
		ifstream in;
		in.open(path.c_str());
		if ( ! in ) {
			SetErrMsg("fopen(%s) : %M", path.c_str());
			return(-1);
		}
		string tiledir;
		in >> tiledir;
		in.close();

		// Construct path to TMS tile directory
		//
		if (IsAbsPath(tiledir)) {
			path = tiledir;
		}
		else {
			string dir = Dirname(path);
			path = Catpath("",dir,tiledir);
		}
		tms_flag = true;
	}


	// If _geoImage already exists make sure the type matches 
	// the cache path (TMS or Tiff). If they don't match delete
	// _geoImage
	//
	if (_geoImage) {
		if ((dynamic_cast<GeoImageGeoTiff *>(_geoImage)) && tms_flag) {
			delete _geoImage;
			_geoImage = NULL;
		}
		if ((dynamic_cast<GeoImageTMS *>(_geoImage)) && ! tms_flag) {
			delete _geoImage;
			_geoImage = NULL;
		}
	}

	// Create an appropriate instance of _geoImage for the cache path
	//
	if (! _geoImage) {
		if (tms_flag) _geoImage = new GeoImageTMS();
		else _geoImage = new GeoImageGeoTiff();
	}

	int rc = _geoImage->Initialize(path, times);
	if (rc < 0) {
		SetErrMsg("GeoImage::Initialize(%s,)", path.c_str());
		return(-1);
	}
	
	return(0);
}

unsigned char *TwoDImageRenderer::_getImage(
	GeoImage *geoimage,
    size_t ts, 
	string proj4StringData,
	vector <double> pcsExtentsDataVec,
    double pcsExtentsImg[4], double geoCornersImg[8], string &proj4StringImg,
    GLsizei &width, GLsizei &height
) const {
	assert(geoimage);
	assert(pcsExtentsDataVec.size() >= 4);

	// Initialize out params
	//
	for (int i=0; i<4; i++) {
		pcsExtentsImg[i] = 0.0;
		geoCornersImg[i] = geoCornersImg[i+4] = 0.0;
	}
	width = height = 0;

	// Ugh. Hardcode maximum image size request
	//
	const int maxWidthReq = 1024;
	const int maxHeightReq = 1024;

	double pcsExtentsData[4];
	for (int i=0; i<4; i++) {
		pcsExtentsData[i] = pcsExtentsDataVec[i];
	}

	size_t my_width, my_height;
	unsigned char *tex;
	if (proj4StringData.empty()) {

		// Data aren't geo-referenced. 
		//
		for (int i=0; i<4; i++) {
			pcsExtentsImg[i] = pcsExtentsData[i];
		}
		tex = geoimage->GetImage(ts, my_width, my_height);
	}
	else {


		tex = geoimage->GetImage(
			ts, pcsExtentsData, proj4StringData, maxWidthReq, maxHeightReq,
			pcsExtentsImg, geoCornersImg, proj4StringImg, 
			my_width, my_height
		);
	}
	width = my_width;
	height = my_height;
	return(tex);
}


int TwoDImageRenderer::_getMeshDisplaced(
	DataMgr *dataMgr,
	GLsizei width,
	GLsizei height,
	const vector <double> &minBox,
	const vector <double> &maxBox
) {

	// Construct the displaced (terrain following) grid using 
	// a map projection, if specified.
	//
	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();
	VizFeatureParams *vizfeature =_visualizer->getActiveVizFeatureParams();

	int refLevel = myParams->GetRefinementLevel();
	int lod = myParams->GetCompressionLevel();

	
	// Get the height variable if one specified
	//
	StructuredGrid* hgtGrid = NULL;
	string hgtVar = myParams->GetHeightVariableName();
	if (! hgtVar.empty()) {
		vector<string>vars;
		vars.push_back(hgtVar);
		int rc = getGrids(
			dataMgr, GetCurrentTimestep(), vars, &refLevel, &lod,  &hgtGrid
		);
	
		if(rc<0){
			MyBase::SetErrMsg(
				"height data unavailable for 2D rendering at timestep %d",
				GetCurrentTimestep()
			);
			return (rc);
		}
	}

	// (Re)allocate space for verts
	//
	size_t vertsSize = width * height * 3;
	GLfloat *dummy = (float *) _sb_verts.Alloc(vertsSize * sizeof(*dummy));
	dummy = (float *) _sb_normals.Alloc(vertsSize * sizeof(*dummy));
	
	int rc;
	if (myParams->GetGeoreferenced()) {
		double defaultZ = minBox.size() > 2 ? minBox[2] : 0.0;
		rc = _getMeshDisplacedGeo(
			dataMgr, hgtGrid, width, height, defaultZ
		);
	}
	else {
		rc = _getMeshDisplacedNoGeo(
			dataMgr, hgtGrid, width, height, minBox, maxBox
		);
	}

	if (hgtGrid) {
		dataMgr->UnlockGrid(hgtGrid);
		delete hgtGrid;
	}

	return(rc);

}

// Compute verts  for displayed, geo-referenced image
//
int TwoDImageRenderer::_getMeshDisplacedGeo(
	DataMgr *dataMgr,
	StructuredGrid *hgtGrid,
	GLsizei width,
	GLsizei height,
	double defaultZ
) {


	// Set up proj.4:
	//
	string proj4String = dataMgr->GetMapProjection();

		
	// Delta between pixels in image in Image PCS coordinates 
	//
	double deltax = (_pcsExtentsImg[2]-_pcsExtentsImg[0]) /(double)(width - 1);
	double deltay = (_pcsExtentsImg[3]-_pcsExtentsImg[1]) /(double)(height - 1);

	// Compute horizontal coordinates in Image PCS space. Ignore
	// vertical coordinate for now.
	//
	GLfloat *verts = (GLfloat *) _sb_verts.GetBuf();
	for (int j = 0; j<height; j++){
	for (int i = 0; i<width; i++){

		verts[j*width*3 + i*3] = _pcsExtentsImg[0] + (i * deltax);
		verts[j*width*3 + i*3+1] = _pcsExtentsImg[1] + (j * deltay);
		verts[j*width*3 + i*3+2] = 0.0;	// ignored

	}
	}

	// apply proj4 to transform the points(in place), converting
	// from Image PCS to Data PCS
	//
	Proj4API proj4;

	int rc = proj4.Transform(
		_proj4StringImg, proj4String, verts, verts+1, NULL, width*height, 3
	);
	if (rc<0){
		MyBase::SetErrMsg("Error in coordinate projection");
		return (-1);
	}

	// Now find vertical coordinate
	//
	double mv = hgtGrid ? hgtGrid->GetMissingValue() : 0.0;
	for (int j = 0; j<height; j++){
	for (int i = 0; i<width; i++){
		float x = verts[j*width*3 + i*3];
		float y = verts[j*width*3 + i*3+1];
		float z = 0.0;

		// Lookup vertical coordinate as a data element from the
		// height variable. Note, missing values are possible if image
		// extents are out side of extents for height variable, or if 
		// height variable itself contains missing values.
		//
		float deltaZ = 0.0;
		if (hgtGrid) {
			deltaZ = hgtGrid->GetValue(x,y,0.0);
			if (deltaZ == mv) deltaZ = 0.0;
		}

		z = deltaZ + defaultZ;

		verts[j*width*3 + i*3+2] = z;

	}
	}
			
	// Take care of any boundary conditions to present meshes with 
	// folds. Still needed?
	//
	conform(verts, width, height);

	return(0);
}

// Compute verts  for displayed, non-georeferenced image
//
int TwoDImageRenderer::_getMeshDisplacedNoGeo(
	DataMgr *dataMgr,
	StructuredGrid *hgtGrid,
	GLsizei width,
	GLsizei height,
	const vector <double> &minExt,
	const vector <double> &maxExt
) {

		
	// Delta between pixels in image in Image PCS coordinates 
	//
	double deltax = (maxExt[0]-minExt[0]) /(double)(width - 1);
	double deltay = (maxExt[1]-minExt[1]) /(double)(height - 1);
	double defaultZ = minExt[2];

	// Compute horizontal coordinates in Image PCS space. Ignore
	// vertical coordinate for now.
	//
	GLfloat *verts = (GLfloat *) _sb_verts.GetBuf();
	double mv = hgtGrid ? hgtGrid->GetMissingValue() : 0.0;
	for (int j = 0; j<height; j++){
		double y = minExt[1] + (j * deltay);

		for (int i = 0; i<width; i++){
			double x = minExt[0] + (i * deltax);
			double z = 0.0;

			verts[j*width*3 + i*3] = x;
			verts[j*width*3 + i*3+1] = y;

			// Lookup vertical coordinate as a data element from the
			// height variable. Note, missing values are possible if image
			// extents are out side of extents for height variable, or if 
			// height variable itself contains missing values.
			//
			float deltaZ = 0;
			if (hgtGrid) {
				deltaZ = hgtGrid->GetValue(x,y,0.0);
				if (deltaZ == mv) deltaZ = 0.0;
			}
			z = deltaZ + defaultZ;

			verts[j*width*3 + i*3+2] = z;

		}
	}

	return(0);
}

int TwoDImageRenderer::_getMeshPlane(
	const vector <double> &minBox,
	const vector <double> &maxBox
) {
	// determine the corners of the textured plane.
	// If it's X-Y (orientation = 2) 
	// If it's X-Z (orientation = 1)
	// If it's Y-Z (orientation = 0)
	//
	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();
	int orient = myParams->GetOrientation();

	size_t vertsSize = 2 * 2 * 3;
	GLfloat *verts = (float *) _sb_verts.Alloc(vertsSize * sizeof(*verts));
	GLfloat *dummy = (float *) _sb_normals.Alloc(vertsSize * sizeof(*dummy));

	if (orient == 2) {	// X-Y
		verts[0] = minBox[0];
		verts[1] = minBox[1];
		verts[2] = minBox[2];

		verts[3] = maxBox[0];
		verts[4] = minBox[1];
		verts[5] = minBox[2];

		verts[6] = minBox[0];
		verts[7] = maxBox[1];
		verts[8] = minBox[2];

		verts[9] = maxBox[0];
		verts[10] = maxBox[1];
		verts[11] = minBox[2];
	}
	else { // X-Z
		SetErrMsg("Orientation == %d  not supported", orient);
		return(-1);
	}

	return(0);
}


// Get the selected horizontal ROI in PCS data coordinates
//
vector <double> TwoDImageRenderer::_getPCSExtentsData() const {

	AnimationParams* myAnimationParams = _visualizer->getActiveAnimationParams();
	size_t ts = myAnimationParams->GetCurrentTimestep();
	TwoDImageParams *myParams = (TwoDImageParams *) getRenderParams();

    // Find box extents for ROI
	//
    vector <double> minBox;
	vector <double> maxBox;
    myParams->GetBox()->GetUserExtents(minBox, maxBox, ts);


	vector <double> pcsExtentsData;
	pcsExtentsData.push_back(minBox[0]);
	pcsExtentsData.push_back(minBox[1]);
	pcsExtentsData.push_back(maxBox[0]);
	pcsExtentsData.push_back(maxBox[1]);

	return(pcsExtentsData);

}

void TwoDImageRenderer::_transformToLocal(
	size_t width, size_t height, 
	const vector <double> &scaleFac
) const {
	AnimationParams* myAnimationParams = _visualizer->getActiveAnimationParams();
	
	size_t ts =  myAnimationParams->GetCurrentTimestep();
    vector<double>minExts,maxExts;
    _dataStatus->GetExtents(ts, minExts, maxExts);

	GLfloat *verts = (GLfloat *) _sb_verts.GetBuf();

	for (int j = 0; j<height; j++){
	for (int i = 0; i<width; i++){
			verts[j*width*3 + i*3] -= minExts[0];
			verts[j*width*3 + i*3+1] -= minExts[1];
			verts[j*width*3 + i*3+2] -= minExts[2];

			verts[j*width*3 + i*3] *= scaleFac[0];
			verts[j*width*3 + i*3+1] *= scaleFac[1];
			verts[j*width*3 + i*3+2] *= scaleFac[2];
	}
	}
}
