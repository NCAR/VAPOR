//************************************************************************
//									*	
//		     Copyright (C)  2018				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		DCUtils.h
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2018
//
//	Description:	Defines the DC free functions.  
//
//  These  functions operate on instances of the DC class.
//
#ifndef DCUTILS_H
#define DCUTILS_H


#include <vector>
#include <string>
#include <map>
#include <vapor/DC.h>

namespace VAPoR {

class NetCDFCollection;

namespace DCUtils {


int CopyAtt(
	const NetCDFCollection &ncdfc, string varname, string attname,
	DC::BaseVar &var
);

int CopyAtt(
	const NetCDFCollection &ncdfc, string varname,
	DC::BaseVar &var
);

};
};

#endif
