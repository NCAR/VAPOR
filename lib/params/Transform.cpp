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
#include <vapor/glutil.h>

using namespace VAPoR;
using namespace Wasp;

const string Transform::_translationTag = "Translation";
const string Transform::_rotationTag = "Rotation";
const string Transform::_scaleTag = "Scale";

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
	_init();
}

Transform::Transform(
	ParamsBase::StateSave *ssave, XmlNode *node
) : ParamsBase(ssave, node)
{ 
	_init();
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
Transform::~Transform()
{
	MyBase::SetDiagMsg("Transform::~Transform() this=%p", this);
}

void Transform::_init() {
	for (int i=0; i<3; i++) {
		_defaultTranslation.push_back(0.0);
		_defaultRotation.push_back(0.0);
		_defaultScale.push_back(1.0);
	}

	SetTranslations(_defaultRotation);
	SetRotations(_defaultRotation);
	SetScales(_defaultScale);
}
