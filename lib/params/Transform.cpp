//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//					
//	File:		Transform.cpp
//
//	Author:		Scott Pearse
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2017
//
//	Description:	Implements the Transform class
//		This class contains the translation, rotation, and scale
//		parameters associated with a loaded data set
//
#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning( disable : 4100 )
#endif

#include <iostream>
#include <vapor/Transform.h>

using namespace VAPoR;
using namespace Wasp;

const string Transform::_translationTag = "Translation";
const string Transform::_rotationTag = "Rotation";
const string Transform::_scaleTag = "Scale";
const string Transform::_originTag = "Origin";
const string Transform::_originInitializedTag = "OriginInitialized";

//
// Register class with object factory!!!
//
static ParamsRegistrar<Transform> registrar(Transform::GetClassType());

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
Transform::Transform(
	ParamsBase::StateSave *ssave
) : ParamsBase(ssave, Transform::GetClassType())
{
}

Transform::Transform(
	ParamsBase::StateSave *ssave, XmlNode *node
) : ParamsBase(ssave, node)
{ 
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
Transform::~Transform()
{
	MyBase::SetDiagMsg("Transform::~Transform() this=%p", this);
}

void Transform::SetScales(const vector<double> scale) {
   SetValueDoubleVec(_scaleTag, "Set scale transform", scale);
}

vector<double> Transform::GetOrigin() const {
	vector <double> defaultv(3,0.0);
	vector<double> origin = GetValueDoubleVec(_originTag, defaultv);
	return origin;
}

void Transform::SetOrigin(const vector<double> origin) {
	SetValueDoubleVec(_originTag, "Set origin for transforms", origin);
	SetOriginInitialized(true);
}

bool Transform::IsOriginInitialized() const
{
	return GetValueLong(_originInitializedTag, false);
}

void Transform::SetOriginInitialized(bool value)
{
	SetValueLong(_originInitializedTag, "Set is origin initialized for transforms", value);
}

