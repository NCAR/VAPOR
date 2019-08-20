//-- HelloRenderer.cpp ----------------------------------------------------------
//   
//                   Copyright (C)  2015
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//----------------------------------------------------------------------------
//
//      File:           HelloRenderer.cpp
//
//      Author:         Alan Norton
//
//      Description:  Implementation of HelloRenderer class
//
//----------------------------------------------------------------------------


#include <vapor/glutil.h>	// Must be included first!!!
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cfloat>

#ifndef WIN32
#include <unistd.h>
#endif

#include <vapor/ParamsMgr.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/HelloRenderer.h>
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>

using namespace VAPoR;
using namespace Wasp;


//
// Register class with object factory!!!
//
static RendererRegistrar<HelloRenderer> registrar(
	HelloRenderer::GetClassType(), HelloParams::GetClassType()
);

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
HelloRenderer::HelloRenderer(
	const ParamsMgr *pm, string winName, string dataSetName,
	string instName, DataMgr *dataMgr
) : Renderer(
	pm, winName, dataSetName, HelloParams::GetClassType(), 
	HelloRenderer::GetClassType(), instName, dataMgr) 
{}
 

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
HelloRenderer::~HelloRenderer()
{
}

int HelloRenderer::_initializeGL(){
	return(0);
}

int HelloRenderer::_paintGL(bool){

	HelloParams *rParams = (HelloParams *) GetActiveParams();

	//Next we need to get a Grid for the data we are rendering.
	Grid *helloGrid;

	//To obtain the Grid, we need the refinement level, variable, LOD, and extents:
	int actualRefLevel = rParams->GetRefinementLevel();
	int lod = rParams->GetCompressionLevel();
	
	//Get the variable name
	string varname = rParams->GetVariableName();

	//Determine the full vdc extents, in order to render 
	// in local user coordinates.



	//Determine the data extents.
	//The extents of data needed are determined by the end points.
	//Get the end points from the Params:
	vector <double> point1 = rParams->GetPoint1();
	vector <double> point2 = rParams->GetPoint2();
	VAssert(point1.size() == point2.size());
	VAssert(point1.size() >= 2 && point1.size() <= 3);

	cout << "helloParams point: " << endl;
 	cout << "point1: " << point1[0] << " " << point1[1] << " " << point1[2] << endl;
 	cout << "point2: " << point2[0] << " " << point2[1] << " " << point2[2] << endl;
	
	//Finally, obtain the Grid of the data for the specified region, at requested refinement and lod,
	//using Renderer::getGrids()
	size_t timestep = rParams->GetCurrentTimestep();

	int rc = DataMgrUtils::GetGrids(
		_dataMgr, timestep, varname, point1, point2, true,
		&actualRefLevel, &lod, &helloGrid
	);
	if (rc<0) {
		return rc;
	}

	//Set the grid to use nearest-point interpolation, to calculate actual (uninterpolated) data max and min
	helloGrid->SetInterpolationOrder(0);

	//In order to sample the data at the user-specified refinement level, need to determine the number of voxels
	//that the line crosses, which requires knowing the underlying grid.  
	//

	size_t nsamples = 100;
	// nsamples is the number of samples along the line.
	// Divide the line into maxvox equal sections, sample the variable at each point along the line, to find 
	// coordinates of min and max value
	double maxval = -DBL_MAX;
	double minval = DBL_MAX;
	vector <double> minPoint, maxPoint;
	for (int i = 0; i<nsamples; i++){
		vector <double> coord;

		for (int j=0; j<point1.size(); j++) {
			coord.push_back(point1[j] + i*(point2[j]-point1[j])/(double)(nsamples-1));
		}

		double sampledVal = helloGrid->GetValue(coord);
		if (sampledVal == helloGrid->GetMissingValue()) continue;
		if (minval > sampledVal){
			minval = sampledVal;
			minPoint = coord;
		}
		if (maxval < sampledVal){
			maxval = sampledVal;
			maxPoint = coord;
		}
	}

	_dataMgr->UnlockGrid(helloGrid);
	
	//Obtain the line width
	float width = (float)rParams->GetLineThickness();

	//Set up lighting and color.  We will use the lighting settings from the viewpoint params for rendering the lines,
	//but lighting will be disabled for rendering the max and min points.
	
	ViewpointParams* vpParams =  _paramsMgr->GetViewpointParams(_winName);
	int nLights = vpParams->getNumLights();
	float fcolor[3];
	rParams->GetConstantColor(fcolor);
	if (nLights == 0) {
		glDisable(GL_LIGHTING);
	}
	else {
		glShadeModel(GL_SMOOTH);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,fcolor);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, vpParams->getExponent());
		//The line geometry will get a white specular color:
		float specColor[4];
		specColor[0]=specColor[1]=specColor[2]=0.8f;
		specColor[3] = 1.f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specColor);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
	}
	glColor3fv(fcolor);
	// glLineWidth(width);

	if (point1.size() != 3) {
		point1.push_back(0.0);
		point2.push_back(0.0);
		minPoint.push_back(0.0);
		maxPoint.push_back(0.0);
	}

	//Calculate the normal vector as orthogonal to the line and projected to the viewer direction
	//To do this, take the cross product of the line direction with the viewer direction, 
	//And then cross the result with the line direction.
	//Find the direction vector along the line and the camera direction

    double m[16];
    vpParams->GetModelViewMatrix(m);

	double posvec[3], upvec[3], dirvec[3];
    bool status = vpParams->ReconstructCamera(m, posvec, upvec, dirvec);
    if (! status) {
        SetErrMsg("Failed to get camera parameters");
        return(-1);
    }

	double lineDir[3], vdir[3], cross[3], normvec[3];
	for (int i = 0; i<3; i++) {
		lineDir[i] = point2[i]-point1[i];
		vdir[i] = dirvec[i];
	}
	float len = vlength(lineDir);
	if (len == 0.f) len = 1.;
	vscale(lineDir, 1./len);
	vcross(vdir, lineDir,cross);
	len = vlength(cross);
	if (len == 0.f) len = 1.;
	vscale(cross, 1./len);
	vcross(cross, lineDir,normvec);
	len = vlength(normvec);
	if (len == 0.f) len = 1.;
	vscale(normvec, 1./len);

	//Now render the line
	//translate to as to render in local user coordinates
	//
	glBegin(GL_LINES);
	glNormal3dv(normvec);
	glVertex3d(point1[0],point1[1],point1[2]);
	glNormal3dv(normvec);
	glVertex3d(point2[0],point2[1],point2[2]);
	glEnd();

	//Then render the Max and Min points:
	glDisable(GL_LIGHTING);
	glPointSize(4.*width);
	//Max will be white
	glColor3f(1.f,1.f,1.f);
	glBegin(GL_POINTS);
	glVertex3d(maxPoint[0],maxPoint[1],maxPoint[2]);
	glEnd();

	//Set min point to be yellow
	glColor3f(1.f, 1.f, 0.f);
	glBegin(GL_POINTS);
	glVertex3d(minPoint[0],minPoint[1],minPoint[2]);
	glEnd();
	
	return 0;
}

