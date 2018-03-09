#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cassert>

#include <vapor/VDCNetCDF.h>
#include <vapor/VDC_c.h>


int main(int argc, char **argv) {

VDC *v = VDC_new();
VDC_Initialize(v, "out.nc", VDC_AccessMode_W, NULL, 0);
VDC_DefineDimension(v, "time", 1);
VDC_DefineDimension(v, "nlon", 3600);
VDC_DefineDimension(v, "nlat", 2400);
VDC_DefineCoordVar(v, "time", NULL, 0, "time", "", VDCDimension_Axis_T, VDC_XType_DOUBLE, false);
{ const char *dims[] = {"nlon", "nlat"}; VDC_DefineDataVar(v, "ULONG", dims, 2, NULL, 0, "", VDC_XType_DOUBLE, false); }
VDC_EndDefine(v);
float *data = (float *) malloc(sizeof(float)*100000);
int rc = VDC_PutVarAtTimeStep(v, 0, "time", -1, data); 
cout << "rc == " << rc << endl;
// VDC_PutVarAtTimeStep: ERROR: code = -1, message = "Error accessing netCDF file "out.nc", nc_put_vara_float : NetCDF: Operation not allowed in define mode -- file (/Users/stas/Doc>
VDC_delete(v);

}
