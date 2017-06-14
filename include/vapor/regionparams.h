//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		regionparams.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		September 2004
//
//	Description:	Defines the RegionParams class.
//		This class supports parameters associted with the
//		region panel, describing the rendering region
//
#ifndef REGIONPARAMS_H
#define REGIONPARAMS_H


#include <vapor/ParamsBase.h>
#include <vapor/Box.h>

namespace VAPoR {

//! \class RegionParams
//! \ingroup Public_Params
//! \brief A class for describing a 3D axis-aligned region in user space.
//! \author Alan Norton
//! \version 3.0
//! \date    February 2014

//! The RegionParams class controls the extents of a 3D box of data for visualization.
//! The DVR, Isosurface and Flow renderers use only the data specified by the current RegionParams.
//! There is a global RegionParams, that 
//! is shared by all windows whose region is set to "global".  There is also
//! a local RegionParams for each window, that users can select whenever there are multiple windows.
//! When local settings are used, they only affect one currently active visualizer.
//! The RegionParams class also has several methods that are useful in setting up data requests from the DataMgr.
//!
class PARAMS_API RegionParams : public ParamsBase {

public: 
 

 RegionParams(
	ParamsBase::StateSave *ssave
 );

 RegionParams(
	ParamsBase::StateSave *ssave, XmlNode *node
 );

 RegionParams(const RegionParams &rhs);

 RegionParams &operator=( const RegionParams& rhs );

 virtual ~RegionParams();


	
	///@}
	//! Method to obtain the current Box defining the region extents
	//! \retval Box* current Box.
	virtual Box* GetBox() const {
		return m_Box;
	}
	

	//! Get the extents extent of the Box, in local coordinates
	//! \param[out] double[6] extents
	//! \param[in] int timestep indicates the current timestep, used only with time-varying extents.
	
#ifdef	DEAD
	void getLocalRegionExtents(double exts[6],int timestep) const {
		GetBox()->GetLocalExtents(exts,timestep);
		return;
	}

	//! Get a center coordinate of the Box, in local coordinates
	//! \param[in] int coord 0,1,2 for x,y,z
	//! \param[in] int timestep indicates the current timestep, used only with time-varying extents.
	//! \retval double value of center for specified coordinate.
	double getLocalRegionCenter(int indx, int timestep) const {
		if (indx < 0 || indx > 2) indx = 0;
		return (0.5*(getLocalRegionMin(indx,timestep)+getLocalRegionMax(indx,timestep)));
	}
#endif

	
#ifdef	DEAD
	//! Provide a vector of the times, useful for time-varying extents
	//! \retval const vector<long> vector of times.
	const vector <long> GetTimes() const {
		return GetBox()->GetTimes();
	}
	
	//! Indicate whether or not the extents vary over time
	//! \retval bool return true if extents are time-varying.
	bool extentsAreVarying() {
		return GetBox()->GetTimes().size()>1;
	}

	//! Insert a specific time in the list of time-varying extents.  Return false if it's already there
	//! \param[in] timestep to be inserted
	//! \retval true if the time was not already in the list.
	bool insertTime(int timestep);

	//! Remove a time from the time-varying timesteps.  Return false if unsuccessful
	//! \param[in] timestep to be removed
	//! \retval false if the time was not already in the list.
	bool removeTime(int timestep);
#endif

#ifdef	DEAD
	//! Provide a vector of all the extents for all times
	//! returns 6 doubles for each time step.
	//! \retval const vector<double> vector of extents. 
	vector <double> GetAllExtents() const {
		return GetBox()->GetLocalExtents();
	}
#endif

#ifdef	DEAD
	//! Provide the domain-defining variables
	//! returns a vector of variable names.
	//! Note that this is an attribute of the global Region params
	//! \retval vector<string> domain-defining variables
	static const vector<string> GetDomainVariables(){ 
		((RegionParams*)_paramsMgr->GetParamsInstance(_regionParamsTag,-1,-1))->GetValueStringVec(_domainVariablesTag);
		return vec;
	}
	//! set the domain-defining variables
	//! Note that this value is stored in the global Region params
	//! \param [in] vector<string> names of domain-defining variables
	//! \retval int 0 if successful
	static int SetDomainVariables(vector<string> varnames){ 
		if (varnames.size() == 0) return -1;
		return ((RegionParams*)_paramsMgr->GetParamsInstance(_regionParamsTag,-1,-1))->SetValueStringVec(_domainVariablesTag, "Set Domain-defining variables",varnames);
	}
#endif

	// Get static string identifier for this params class
	//
	static string GetClassType() {
		return("RegionParams");
	}


private:

	Box *m_Box;

	static const string _domainVariablesTag;

	void _init();
	void _reconcile();
	
};

};
#endif //REGIONPARAMS_H 
