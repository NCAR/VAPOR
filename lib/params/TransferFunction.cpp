//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		transferfunction.cpp//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2004
//
//	Description:	Implements the TransferFunction class
//		This is the mathematical definition of the transfer function
//		It is defined in terms of floating point coordinates, converted
//		into a mapping of quantized values (LUT) with specified domain at
//		rendering time.  Interfaces to the TFEditor and DvrParams classes.
//		The TransferFunction deals with an mapping on the interval [0,1]
//		that is remapped to a specified interval by the user.
//
//----------------------------------------------------------------------------

#ifdef WIN32
    #pragma warning(disable : 4251 4100)
#endif
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdarg>
#include <cassert>
#include <algorithm>

#include <vapor/TransferFunction.h>

using namespace VAPoR;
using namespace Wasp;

//----------------------------------------------------------------------------
// Static member initialization.  Acceptable tags in transfer function output
// Sooner or later we may want to support
//----------------------------------------------------------------------------

//
// Register class with object factory!!!
//
static ParamsRegistrar<TransferFunction> registrar(TransferFunction::GetClassType());

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
TransferFunction::TransferFunction(ParamsBase::StateSave *ssave) : MapperFunction(ssave, TransferFunction::GetClassType()) {}

TransferFunction::TransferFunction(ParamsBase::StateSave *ssave, XmlNode *node) : MapperFunction(ssave, node) {}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
TransferFunction::~TransferFunction() { MyBase::SetDiagMsg("TransferFunction::~TransferFunction() this=%p", this); }

//----------------------------------------------------------------------------
// Create a transfer function by parsing a file.
//----------------------------------------------------------------------------
int TransferFunction::LoadFromFile(string path)
{
    XmlParser xmlparser;

    XmlNode *node = GetNode();

    int rc = xmlparser.LoadFromFile(node, path);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to read file %s : %M", path.c_str());
        return (-1);
    }
    return (0);
}

//----------------------------------------------------------------------------
// Save the transfer function to a file.
//----------------------------------------------------------------------------
int TransferFunction::SaveToFile(string path)
{
    ofstream out(path);
    if (!out) {
        MyBase::SetErrMsg("Failed to open file %s : %M", path.c_str());
        return (-1);
    }

    XmlNode *node = GetNode();

    out << *node;

    if (out.bad()) {
        MyBase::SetErrMsg("Failed to write file %s : %M", path.c_str());
        return (-1);
    }
    out.close();

    return (0);
}
