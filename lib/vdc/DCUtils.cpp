//************************************************************************
//									*
//		     Copyright (C)  2018				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		DataMgrUtils.cpp
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2018
//
//	Description:	Implements the DataMgrUtils free functions
//
#ifdef WIN32
#pragma warning(disable : 4251 4100)
#endif

#include <iostream>
#include <cassert>
#include <algorithm>


#include <vapor/NetCDFCollection.h>
#include <vapor/DCUtils.h>


using namespace VAPoR;
using namespace Wasp;


int DCUtils::CopyAtt(
    const NetCDFCollection &ncdfc, string varname, string attname,
	DC::BaseVar &var
) {

	int nctype = ncdfc.GetAttType(varname, attname);

    if (nctype == NC_INT64) {

        vector<long> values;
        ncdfc.GetAtt(varname, attname, values);

		DC::Attribute attr(attname, DC::INT64, values);
		var.SetAttribute(attr);

    } else if (nctype == NC_DOUBLE) {

        vector<double> values;
        ncdfc.GetAtt(varname, attname, values);

		DC::Attribute attr(attname, DC::DOUBLE, values);
		var.SetAttribute(attr);

    } else if (nctype == NC_CHAR) {

        string values;
        ncdfc.GetAtt(varname, attname, values);

		DC::Attribute attr(attname, DC::TEXT, values);
		var.SetAttribute(attr);

    } else {
        Wasp::MyBase::SetErrMsg(
			"Invalid attribute : %s.%s", varname.c_str(), attname.c_str()
		);
        return -1;
    }
    return 0;
}

int DCUtils::CopyAtt(
    const NetCDFCollection &ncdfc, string varname, 
	DC::BaseVar &var
) {
	vector <string> attNames = ncdfc.GetAttNames(varname);

	for (int i=0; i<attNames.size(); i++) {
		int rc = CopyAtt(ncdfc, varname, attNames[i], var);
		if (rc<0) return(rc);
	}
	return(0);
}
