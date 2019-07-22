//************************************************************************
//         *
//       Copyright (C)  2015    *
//     University Corporation for Atmospheric Research   *
//       All Rights Reserved    *
//         *
//************************************************************************/
//
// File:  OSPRayParams.h
//
// Authors:
//   National Center for Atmospheric Research
//   PO 3000, Boulder, Colorado
//
//  Description: Defines the OSPRayParams class.
//
#pragma once

#include <vector>
#include <vapor/ParamsBase.h>

namespace VAPoR {

//! \class OSPRayParams
//! \ingroup Public_Params
//!
class OSPRayParams : public VAPoR::ParamsBase {
public: 

	OSPRayParams(VAPoR::ParamsBase::StateSave *ssave);
	OSPRayParams(VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node);
	OSPRayParams(const OSPRayParams &rhs);
	OSPRayParams &operator=(const OSPRayParams& rhs);

	~OSPRayParams();

	int GetNumThreads() const;
	void SetNumThreads(int num);
    int GetAOSamples() const;
    void SetAOSamples(int n);

	static string GetClassType()
	{
		return(_classType);
	}

public:

	static const string _classType;
	static const string _shortName;
	static const string _numThreadsTag;
	static const string _aoSamplesTag;
    static const string _samplesPerPixelTag;

private:
	void _init();

	string _settingsPath;
};

}
