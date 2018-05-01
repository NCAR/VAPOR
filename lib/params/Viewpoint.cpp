//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Viewpoint.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2005
//
//	Description:	Implements the Viewpoint class
//		This class contains the parameters associated with one viewpoint
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <iostream>

#include <vapor/Viewpoint.h>
#include <vapor/glutil.h>

using namespace VAPoR;
using namespace Wasp;

const string Viewpoint::_modelViewMatrixTag = "ModelViewMatrix";
const string Viewpoint::_projectionMatrixTag = "ProjectionMatrix";
double       Viewpoint::_defaultModelViewMatrix[] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
double       Viewpoint::_defaultProjectionMatrix[] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};

//
// Register class with object factory!!!
//
static ParamsRegistrar<Viewpoint> registrar(Viewpoint::GetClassType());

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
Viewpoint::Viewpoint(ParamsBase::StateSave *ssave) : ParamsBase(ssave, Viewpoint::GetClassType()) { _init(); }

Viewpoint::Viewpoint(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node) {}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
Viewpoint::~Viewpoint() { MyBase::SetDiagMsg("Viewpoint::~Viewpoint() this=%p", this); }

void Viewpoint::_init()
{
    double proj[] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};

    double modelview[] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};

    SetProjectionMatrix(proj);
    SetModelViewMatrix(modelview);
}

void Viewpoint::GetModelViewMatrix(double m[16]) const
{
    vector<double> defaultv;
    for (int i = 0; i < 16; i++) defaultv.push_back(_defaultModelViewMatrix[i]);

    vector<double> val = GetValueDoubleVec(_modelViewMatrixTag, defaultv);
    assert(val.size() == 16);

    for (int i = 0; i < val.size(); i++) m[i] = val[i];
}

void Viewpoint::SetModelViewMatrix(const double m[16])
{
    vector<double> val;
    for (int i = 0; i < 16; i++) val.push_back(m[i]);

#ifdef VAPOR3_0_0_ALPHA
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) { cout << m[j * 4 + i] << " "; }
        cout << endl;
    }
    cout << endl;
#endif

    SetValueDoubleVec(_modelViewMatrixTag, "Model view matrix", val);
}

void Viewpoint::GetProjectionMatrix(double m[16]) const
{
    vector<double> defaultv;
    for (int i = 0; i < 16; i++) defaultv.push_back(_defaultProjectionMatrix[i]);

    vector<double> val = GetValueDoubleVec(_projectionMatrixTag, defaultv);
    assert(val.size() == 16);

    for (int i = 0; i < val.size(); i++) m[i] = val[i];
}

void Viewpoint::SetProjectionMatrix(const double m[16])
{
    vector<double> val;
    for (int i = 0; i < 16; i++) val.push_back(m[i]);
    SetValueDoubleVec(_projectionMatrixTag, "Projection matrix", val);
}

bool Viewpoint::ReconstructCamera(const double m[16], double position[3], double upVec[3], double viewDir[3]) const
{
    double minv[16];

    int rc = minvert((double *)m, minv);
    if (rc < 0) return (false);

    vscale(minv + 8, -1.0);

    for (int i = 0; i < 3; i++) {
        position[i] = minv[12 + i];    // position vector is minv[12..14]
        upVec[i] = minv[4 + i];        // up vector is minv[4..6]
        viewDir[i] = minv[8 + i];      // view direction is minv[8..10]
    }
    vnormal(upVec);
    vnormal(viewDir);

    return (true);
}
