//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//					
//	File:		ViewpointParams.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		August 2004
//
//	Description:	Implements the ViewpointParams class
//		This class contains the parameters associated with viewpoint and lights
//
#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning( disable : 4100 )
#endif

#include <iostream>
#include <algorithm>

#include <vapor/ViewpointParams.h>



double VAPoR::ViewpointParams::_defaultLightDirection[3][4] = {
	{0.f, 0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f, 0.f}
};

int VAPoR::ViewpointParams::_defaultNumLights = 1;
double VAPoR::ViewpointParams::_defaultDiffuseCoeff[3] = {0.8f, 0.8f, 0.8f};
double VAPoR::ViewpointParams::_defaultSpecularCoeff[3] = {0.3f, 0.3f, 0.3f};
double VAPoR::ViewpointParams::_defaultAmbientCoeff = 0.1f;
double VAPoR::ViewpointParams::_defaultSpecularExp = 20.f;


using namespace VAPoR;
using namespace Wasp;

const string ViewpointParams::UseCustomFramebufferTag    = "UseCustomFramebuffer";
const string ViewpointParams::CustomFramebufferWidthTag  = "CustomFramebufferWidth";
const string ViewpointParams::CustomFramebufferHeightTag = "CustomFramebufferHeight";

const string ViewpointParams::_viewPointsTag = "Viewpoints";
const string ViewpointParams::_transformsTag = "Transforms";
const string ViewpointParams::_currentViewTag = "CurrentViewpoint";
const string ViewpointParams::_lightDirectionsTag = "LightDirections";
const string ViewpointParams::_diffuseCoeffTag = "DiffuseCoefficients";
const string ViewpointParams::_specularCoeffTag = "SpecularCoefficients";
const string ViewpointParams::_specularExpTag = "SpecularExponent";
const string ViewpointParams::_ambientCoeffTag = "AmbientCoefficient";
const string ViewpointParams::_numLightsTag = "NumLights";
const string ViewpointParams::m_windowSizeTag = "WindowSize";
const string ViewpointParams::m_stretchFactorsTag = "StretchFactors";
const string ViewpointParams::m_fieldOfView = "FieldOfView";
const string ViewpointParams::_orthoProjectionSizeTag = "OrthoProjectionSize";
const string ViewpointParams::_projectionTypeTag = "ProjectionType";



//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
ViewpointParams::ViewpointParams(
    ParamsBase::StateSave *ssave
 ) : ParamsBase(ssave, ViewpointParams::GetClassType())
{


	_init();

	m_VPs = new ParamsContainer(ssave, _viewPointsTag);
	m_VPs->SetParent(this);

	Viewpoint currentVP(ssave);
	m_VPs->Insert(&currentVP, _currentViewTag);

	_transforms = new ParamsContainer(ssave, _transformsTag);
	_transforms->SetParent(this);


}

ViewpointParams::ViewpointParams(
    ParamsBase::StateSave *ssave, XmlNode *node
) : ParamsBase(ssave, node)
{

	// If node isn't tagged correctly we correct the tag and reinitialize
	// from scratch;
	//
	if (node->GetTag() != ViewpointParams::GetClassType()) {
		node->SetTag(ViewpointParams::GetClassType());
		_init();
	}

	if (node->HasChild(_viewPointsTag)) {
		m_VPs = new ParamsContainer(ssave,node->GetChild(_viewPointsTag));
	}
	else {
		// Node doesn't contain a viewpoint container
		//
		m_VPs = new ParamsContainer(ssave, _viewPointsTag);
		m_VPs->SetParent(this);
	
	}

	if (node->HasChild(_transformsTag)) {
		_transforms = new ParamsContainer(ssave, node->GetChild(_transformsTag));
	}
	else {
		// Node doesn't contain a transforms container
		//
		_transforms = new ParamsContainer(ssave, _transformsTag);
		_transforms->SetParent(this);
	}
}

ViewpointParams::ViewpointParams(const ViewpointParams &rhs
) : ParamsBase(rhs) {

    m_VPs = new ParamsContainer(*(rhs.m_VPs));
	_transforms = new ParamsContainer(*(rhs._transforms));

	
}

ViewpointParams &ViewpointParams::operator=( const ViewpointParams& rhs ) {
	if (m_VPs) delete m_VPs;

	ParamsBase::operator=(rhs);

    m_VPs = new ParamsContainer(*(rhs.m_VPs));
	_transforms = new ParamsContainer(*(rhs._transforms));

    return(*this);
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
ViewpointParams::~ViewpointParams()
{
    MyBase::SetDiagMsg("ViewpointParams::~ViewpointParams() this=%p", this);

	if (m_VPs) {
		delete m_VPs;
		m_VPs = NULL;
	}
	if (_transforms) {
		delete _transforms;
		_transforms = NULL;
	}
}


//Reinitialize viewpoint settings, to center view on the center of full region.
//(this is starting state)
void ViewpointParams::_init() {



	setNumLights(_defaultNumLights);
	setExponent(_defaultSpecularExp);
	setAmbientCoeff(_defaultAmbientCoeff);

	for (int i=0; i<3; i++) {
		setDiffuseCoeff(i, _defaultDiffuseCoeff[i]);
	}

	for (int i=0; i<3; i++) {
		setSpecularCoeff(i, _defaultSpecularCoeff[i]);
	}

	for (int light = 0; light < 3; light++) {
	for (int i=0; i<4; i++) {
		setLightDirection(light, i, _defaultLightDirection[light][i]);
	}
	}

	//SetStretchFactors(vector <double>(3,1.0));
	

	SetWindowSize(100,100);

    SetValueLong(UseCustomFramebufferTag, UseCustomFramebufferTag, false);
    SetValueLong(CustomFramebufferWidthTag, CustomFramebufferWidthTag, 1920);
    SetValueLong(CustomFramebufferHeightTag, CustomFramebufferHeightTag, 1080);
}

Transform* ViewpointParams::GetTransform(string dataSetName) {
	if (_transforms->GetParams(dataSetName) == NULL) {
		Transform newTransform(_ssave);
		_transforms->Insert(&newTransform, dataSetName);
	}
	return (Transform *) _transforms->GetParams(dataSetName);
}

double ViewpointParams::getLightDirection(int lightNum, int dir) const {
	if (lightNum < 0 || lightNum > 2) lightNum = 0;
	if (dir < 0 || dir > 3) dir = 0;

	vector <double> defaultv;
	for (int light = 0; light < 3; light++) {
	for (int i=0; i<4; i++) {
		defaultv.push_back(_defaultLightDirection[light][i]);
	}
	}

	vector <double> v = GetValueDoubleVec(_lightDirectionsTag, defaultv);

	return(v[lightNum*4+dir]);
}

void ViewpointParams::setLightDirection(
	int lightNum, int dir, double val
) {

	if (lightNum < 0 || lightNum > 2) lightNum = 0;
	if (dir < 0 || dir > 3) dir = 0;

	vector <double> defaultv;
	for (int light = 0; light < 3; light++) {
	for (int i=0; i<4; i++) {
		defaultv.push_back(_defaultLightDirection[light][i]);
	}
	}
	vector <double> v = GetValueDoubleVec(_lightDirectionsTag, defaultv);

	v[lightNum*4+dir] = val;

	SetValueDoubleVec(_lightDirectionsTag,"Set light direction",v);
}

double ViewpointParams::getDiffuseCoeff(int lightNum) const{
	if (lightNum < 0 || lightNum > 2) lightNum = 0;

	vector<double> defaultv;
	for (int i = 0; i<3; i++) defaultv.push_back(_defaultDiffuseCoeff[i]);

	double v = GetValueDoubleVec(_diffuseCoeffTag,defaultv)[lightNum];

	return(v);
}

double ViewpointParams::getSpecularCoeff(int lightNum) const{
	if (lightNum < 0 || lightNum > 2) lightNum = 0;

	vector<double> defaultv;
	for (int i = 0; i<3; i++) defaultv.push_back(_defaultSpecularCoeff[i]);

	double v = GetValueDoubleVec(_specularCoeffTag,defaultv)[lightNum];

	return(v);
}

void ViewpointParams::setDiffuseCoeff(int lightNum, double val) {
	if (lightNum < 0 || lightNum > 2) lightNum = 0;

	vector<double> defaultv;
	for (int i = 0; i<3; i++) defaultv.push_back(_defaultDiffuseCoeff[i]);

	vector <double> v = GetValueDoubleVec(_diffuseCoeffTag,defaultv);

	v[lightNum] = val;

	SetValueDoubleVec(_diffuseCoeffTag,"Set diffuse coefficient",v);
}

void ViewpointParams::setSpecularCoeff(int lightNum, double val) {
	if (lightNum < 0 || lightNum > 2) lightNum = 0;

	vector<double> defaultv;
	for (int i = 0; i<3; i++) defaultv.push_back(_defaultSpecularCoeff[i]);

	vector <double> v = GetValueDoubleVec(_specularCoeffTag,defaultv);

	v[lightNum] = val;

	SetValueDoubleVec(_specularCoeffTag,"Set specular coefficient",v);
}

void ViewpointParams::SetCurrentViewpoint(Viewpoint* newVP) {
	m_VPs->Insert(newVP, _currentViewTag);
}

void ViewpointParams::SetWindowSize(size_t width, size_t height) {
	vector <long> v;
	v.push_back(width);
	v.push_back(height);
	SetValueLongVec(m_windowSizeTag,"Set window width and height", v);
}

void ViewpointParams::GetWindowSize(size_t &width, size_t &height) const {
	vector <long> defaultv(2,100);
	vector <long> val = GetValueLongVec(m_windowSizeTag,defaultv);
	width = val[0];
	height = val[1];
}

void ViewpointParams::SetFOV(float v) {
	if (v<5) v=5;
	if (v>90) v=90;
	SetValueDouble(m_fieldOfView, "Set Field of View", v);
}

double ViewpointParams::GetFOV() const {
	double defaultv = 45.0;
	double v = GetValueDouble(m_fieldOfView, defaultv);
	if (v<5) v=5;
	if (v>90) v=90;
	return(v);
}

void ViewpointParams::SetOrthoProjectionSize(float f) {
    SetValueDouble(_orthoProjectionSizeTag, "Set orthographic projection size", f);
}

double ViewpointParams::GetOrthoProjectionSize() const {
    double defaultv = 1.0;
    double v = GetValueDouble(_orthoProjectionSizeTag, defaultv);
    return v;
}

void ViewpointParams::SetProjectionType(ViewpointParams::ProjectionType type) {
    SetValueLong(_projectionTypeTag, "Set projection type", type);
}

ViewpointParams::ProjectionType ViewpointParams::GetProjectionType() const {
    const long defaultV = Perspective;
    return (ViewpointParams::ProjectionType)GetValueLong(_projectionTypeTag, defaultV);
}

vector<double> ViewpointParams::GetStretchFactors() const {
	vector<double> defaultvec(3,1.);
	vector <double> val = GetValueDoubleVec(m_stretchFactorsTag, defaultvec);
	for (int i=0; i<val.size(); i++) {
		if (val[i] == 0) val[i] = 1.0;
		if (val[i] < 0) val[i] *= -1.0;
	}
	return(val);
}

#ifdef VAPOR3_0_0_ALPHA
void ViewpointParams::SetStretchFactors(vector<double> val) {
	vector<double> defaultv(3,1.);
	if (val.size() != defaultv.size()) val = defaultv;
	for (int i=0; i<val.size(); i++) {
		if (val[i] == 0) val[i] = 1.0;
		if (val[i] < 0) val[i] *= -1.0;
	}
	SetValueDoubleVec(m_stretchFactorsTag, "Scene scaling factors", val);
}
#endif



#ifdef	VAPOR3_0_0_ALPHA
//Rescale viewing parameters when the scene is rescaled by factor
void ViewpointParams::rescale (vector<double> scaleFac){
	double vtemp[3], vtemph[3];
	vector<double> vtemp2, vtemp2h;
	Viewpoint* vp = getCurrentViewpoint();
	Viewpoint* vph = getHomeViewpoint();
	vector<double> vps = vp->getCameraPosLocal();
	vector<double> vctr = vp->getRotationCenterLocal();
	vector<double> vpsh = vph->getCameraPosLocal();
	vector<double> vctrh = vph->getRotationCenterLocal();
	for (int i = 0; i<3; i++) {
		vtemp[i] = vps[i]-vctr[i];
		vtemph[i] = vpsh[i]-vctrh[i];
	}
	
	//Want to move the camera in or out, based on scaling in directions orthogonal to view dir.
	for (int i = 0; i<3; i++){
		vtemp[i] /= scaleFac[i];
		vtemph[i] /= scaleFac[i];
	}
	for (int i = 0; i<3; i++) {
		vtemp2.push_back(vtemp[i] + vctr[i]);
		vtemp2h.push_back(vtemph[i] + vctrh[i]);
	}
	
	vp->setCameraPosLocal(vtemp2);
	vph->setCameraPosLocal(vtemp2h);

	return;
}

#endif
	




#ifdef	VAPOR3_0_0_ALPHA
double ViewpointParams::GetCurrentViewDiameter(
	vector <double> stretchFactors
) const {
	double campos[3],rotctr[3];
	getStretchedCamPosLocal(stretchFactors, campos);
	getStretchedRotCtrLocal(stretchFactors, rotctr);
	return (2.*vdist(campos,rotctr)*tan(22.5*M_PI/180.));
}
#endif

