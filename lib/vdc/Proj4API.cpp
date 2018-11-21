
#include <iostream>
#include <proj_api.h>
#include <vapor/GetAppPath.h>
#include <vapor/Proj4API.h>

using namespace VAPoR;
using namespace Wasp;


Proj4API::Proj4API() {
	_pjSrc = NULL;
	_pjDst = NULL;

	vector <string> paths;
	paths.push_back("proj");
	string path =  GetAppPath("VAPOR", "share", paths).c_str();
	if (! path.empty()) {
#ifdef WIN32
		path = "PROJ_LIB="+path;
 		int rc = _putenv(path.c_str());
		if (rc!=0)
 			MyBase::SetErrMsg("putenv failed on PROJ_LIB setting");
#else
		setenv("PROJ_LIB", path.c_str(), 1);
#endif
	}
}

Proj4API::~Proj4API() {
	if (_pjSrc) pj_free(_pjSrc);
	if (_pjDst) pj_free(_pjDst);
}

int Proj4API::_Initialize(
	string srcdef, string dstdef,
	void **pjSrc, void **pjDst
) const {
	*pjSrc = NULL;
	*pjDst = NULL;

	if (! srcdef.empty()) {
		*pjSrc = pj_init_plus(srcdef.c_str());
		if (! *pjSrc) {
			SetErrMsg("pj_init_plus(%s) : %s",srcdef.c_str(),ProjErr().c_str());
			return(-1);
		}
	}

	if (! dstdef.empty()) {
		*pjDst = pj_init_plus(dstdef.c_str());
		if (! *pjDst) {
			SetErrMsg("pj_init_plus(%s) : %s",dstdef.c_str(),ProjErr().c_str());
			return(-1);
		}
	}

	//
	// If either the source or destination definition string is 
	// not provided - but not both - generate a "latlong" conversion
	//
	if (srcdef.empty() && ! dstdef.empty()) {
		*pjSrc = pj_latlong_from_proj(*pjDst);
		if (! *pjSrc) {
			SetErrMsg("pj_latlong_from_proj() : %s", ProjErr().c_str());
			return(-1);
		}
	}
	else if (! srcdef.empty() && dstdef.empty()) {
		*pjDst = pj_latlong_from_proj(*pjSrc);
		if (! *pjDst) {
			SetErrMsg("pj_latlong_from_proj() : %s", ProjErr().c_str());
			return(-1);
		}
	}
	else {
		// NULL transform. Transforms will be no-ops
	}
	return(0);
}

int Proj4API::Initialize(string srcdef, string dstdef) {

	if (_pjSrc) pj_free(_pjSrc);
	if (_pjDst) pj_free(_pjDst);
	_pjSrc = NULL;
	_pjDst = NULL;

	return(_Initialize(srcdef, dstdef, &_pjSrc, &_pjDst));
}

bool Proj4API::IsLatLonSrc() const {
	if (! _pjSrc) return(false);

	return((bool) pj_is_latlong(_pjSrc));
}

bool Proj4API::IsLatLonDst() const {
	if (! _pjDst) return(false);

	return((bool) pj_is_latlong(_pjDst));
}

bool Proj4API::IsGeocentSrc() const {
	if (! _pjSrc) return(false);

	return((bool) pj_is_geocent(_pjSrc));
}

bool Proj4API::IsGeocentDst() const {
	if (! _pjDst) return(false);

	return((bool) pj_is_geocent(_pjDst));
}

string Proj4API::GetSrcStr() const {
	if (! _pjSrc) return("");

	return((string) pj_get_def(_pjSrc, 0));
}

string Proj4API::GetDstStr() const {
	if (! _pjDst) return("");

	return((string) pj_get_def(_pjDst, 0));
}

int Proj4API::Transform(double *x, double *y, size_t n, int offset) const {

	return(Proj4API::Transform(x,y,NULL,n,offset));
}

int Proj4API::_Transform(
	void *pjSrc, void *pjDst, 
	double *x, double *y, double *z, size_t n, int offset
) const {

	// no-op
	//
	if (pjSrc == NULL || pjDst == NULL) return(0);

	//
	// Convert from degrees to radians if source is in 
	// geographic coordinates
	//
	if (pj_is_latlong(pjSrc)) {
		if (x) {
			for (size_t i=0; i<n; i++) {
				x[i * (size_t) offset] *= DEG_TO_RAD;
			}
		}
		if (y) {
			for (size_t i=0; i<n; i++) {
				y[i * (size_t) offset] *= DEG_TO_RAD;
			}
		}
		if (z) {
			for (size_t i=0; i<n; i++) {
				z[i * (size_t) offset] *= DEG_TO_RAD;
			}
		}
	}

	int rc = pj_transform(pjSrc, pjDst, n, offset, x, y, NULL);
	if (rc != 0) {
		SetErrMsg("pj_transform() : %s", ProjErr().c_str());
		return(-1);
	}

	//
	// Convert from radians degrees if destination is in 
	// geographic coordinates
	//
	if (pj_is_latlong(pjDst)) {
		if (x) {
			for (size_t i=0; i<n; i++) {
				x[i * (size_t) offset] *= RAD_TO_DEG;
			}
		}
		if (y) {
			for (size_t i=0; i<n; i++) {
				y[i * (size_t) offset] *= RAD_TO_DEG;
			}
		}
		if (z) {
			for (size_t i=0; i<n; i++) {
				z[i * (size_t) offset] *= RAD_TO_DEG;
			}
		}
	}
	return(0);
}


int Proj4API::Transform(
	double *x, double *y, double *z, size_t n, int offset
) const {

	return(_Transform(_pjSrc, _pjDst, x, y, z, n, offset));
}

int Proj4API::Transform(float *x, float *y, size_t n, int offset) const {

	return(Proj4API::Transform(x,y,NULL,n,offset));
}

int Proj4API::_Transform(
	void *pjSrc, void *pjDst, 
	float *x, float *y, float *z, size_t n, int offset
) const {
	double *xd = NULL;
	double *yd = NULL;
	double *zd = NULL;

	if (x) {
		xd = new double[n];
		for (size_t i = 0; i<n; i++) xd[i] = x[i*offset];
	}
	if (y) {
		yd = new double[n];
		for (size_t i = 0; i<n; i++) yd[i] = y[i*offset];
	}
	if (z) {
		zd = new double[n];
		for (size_t i = 0; i<n; i++) zd[i] = z[i*offset];
	}

	int rc = _Transform(pjSrc, pjDst, xd,yd,zd,n,1);

	if (xd) {
		for (size_t i = 0; i<n; i++) x[i*offset] = xd[i];
		delete [] xd;
	}
	if (yd) {
		for (size_t i = 0; i<n; i++) y[i*offset] = yd[i];
		delete [] yd;
	}
	if (zd) {
		for (size_t i = 0; i<n; i++) z[i*offset] = zd[i];
		delete [] zd;
	}
	return(rc);
}

int Proj4API::Transform(
	float *x, float *y, float *z, size_t n, int offset
) const {

	return(Proj4API::_Transform(_pjSrc, _pjDst, x, y, z, n, offset));
}

int Proj4API::Transform(
	string srcdef, string dstdef,
	double *x, double *y, double *z, size_t n, int offset
) const {
	void *pjSrc = NULL;
	void *pjDst = NULL;

	int rc = _Initialize(srcdef, dstdef, &pjSrc, &pjDst);
	if (rc<0) return(rc);

	return(_Transform(pjSrc, pjDst, x,y,z,n,offset));

	return(0);
}

int Proj4API::Transform(
	string srcdef, string dstdef,
	float *x, float *y, float *z, size_t n, int offset
) const {
	void *pjSrc = NULL;
	void *pjDst = NULL;

	int rc = _Initialize(srcdef, dstdef, &pjSrc, &pjDst);
	if (rc<0) return(rc);

	return(_Transform(pjSrc, pjDst, x,y,z,n,offset));

	return(0);
}

string Proj4API::ProjErr() const {
	return (pj_strerrno(*pj_get_errno_ref()));
}

void Proj4API::Clamp(double *x, double *y, size_t n, int offset) const {
	double minx, miny, maxx, maxy;

	string projstring = GetSrcStr();

	if (IsLatLonSrc()) {
		minx = -180.0;
		miny = -90.0;
		maxx = 180.0;
		maxy = 90.0;
	}
	else if (std::string::npos != projstring.find("proj=eqc")) {
		minx = -20037508.3427892;
		miny = -10018754.1713946;
		maxx = -minx;
		maxy = -miny;
	}
	else if (std::string::npos != projstring.find("proj=merc")) {
		minx = -20037508.340;
		miny = minx;
		maxx = -minx;
		maxy = -miny;
	}
	else {
			return;	// unknown  projectoin
	}

	for (int i=0; i<n; i++) {
		if (x[i*offset] < minx) x[i*offset] = minx;
		if (y[i*offset] < miny) y[i*offset] = miny;
		if (x[i*offset] > maxx) x[i*offset] = maxx;
		if (y[i*offset] > maxy) y[i*offset] = maxy;
	}
}
