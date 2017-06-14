#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cassert>

#include <vapor/VDCNetCDF.h>

using namespace Wasp;
using namespace VAPoR;


int main(int argc, char **argv) {

	MyBase::SetErrMsgFilePtr(stderr);

	string vdc_master = "master.nc";

	VDCNetCDF	vdc;
	vdc.Initialize(vdc_master, VDC::W);

	//
	// Define dimension names and implicity define 1d coord vars
	//
	vdc.DefineDimension("lon", 256, 0);
	vdc.DefineDimension("lat", 128, 1);
	vdc.DefineDimension("lev", 40, 2);
	vdc.DefineDimension("t", 100, 3);

	vector <string> dimnames;
	dimnames.push_back("lon");
	dimnames.push_back("lat");
	vdc.DefineCoordVar(
		"geolon", dimnames, "degrees_E", 0, VDC::FLOAT, false
	);
	vdc.DefineCoordVar(
		"geolat", dimnames, "degrees_N", 1, VDC::FLOAT, false
	);
	

	vector <string> coordvars;
	coordvars.push_back("geolon");
	coordvars.push_back("geolat");
	coordvars.push_back("t");
	dimnames.push_back("t");
	vdc.DefineDataVar("u", dimnames, coordvars, "m/s", VDC::FLOAT, true);
	vdc.DefineDataVar("v", dimnames, coordvars, "m/s", VDC::FLOAT, true);

	vector <double> values;
	values.push_back(0.0);
	values.push_back(1.0);
	vdc.PutAtt("u", "floatatt", VDC::FLOAT, values);

	vdc.EndDefine();

	cout << vdc;

	string varname = "u";
	size_t ts = 0;
	int lod = -1;
	string path;
	size_t file_ts;

	vdc.GetPath(varname, ts, lod, path, file_ts);
	cout << "GetPath(" << varname << ", " << ts << ", " << lod << ") : " << path << ", " << file_ts << endl;

	ts = 99;
	vdc.GetPath(varname, ts, lod, path, file_ts);
	cout << "GetPath(" << varname << ", " << ts << ", " << lod << ") : " << path << ", " << file_ts << endl;
	
	
	varname = "geolon";
	vdc.GetPath(varname, ts, lod, path, file_ts);
	cout << "GetPath(" << varname << ", " << ts << ", " << lod << ") : " << path << ", " << file_ts << endl;
	

	exit(0);
}
