//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		DataStatus.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2006
//
//	Description:	Implements the DataStatus class
//
#ifdef WIN32
#pragma warning(disable : 4251 4100)
#endif

#include <iostream>
#include <cassert>
#include <cfloat>
#include <algorithm>


#include <vapor/DataStatus.h>
#include <vapor/DataMgr.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/ParamsMgr.h>
#include <vapor/Proj4API.h>


using namespace VAPoR;
using namespace Wasp;




//Default constructor
//Whether or not it exists on disk, what's its max and min
//What resolutions are available.
//
DataStatus::DataStatus(size_t cacheSize, int nThreads) {

	_cacheSize = cacheSize;
	_nThreads = nThreads;

	_dataMgrs.clear();
	_timeCoords.clear();
	_timeMap.clear();


	reset_time();
}

int DataStatus::Open(
	const std::vector <string> &files, string name, string format
) {

	if (name.empty()) name = "DataSet1";

	Close(name);

	DataMgr *dataMgr = new DataMgr(format, _cacheSize, _nThreads);

	int rc = dataMgr->Initialize(files);
	if (rc < 0) {
		delete dataMgr;
		return(-1);
	}

	_dataMgrs[name] = dataMgr;

	reset_time();

	return(0);
}

void DataStatus::Close(string name) {

	if (name.empty()) name = "DataSet1";

	map <string, DataMgr *>::iterator itr;
	if ((itr = _dataMgrs.find(name)) != _dataMgrs.end()) {
		if (itr->second) delete itr->second;
		_dataMgrs.erase(itr);
	}


	reset_time();
}

//const DataMgr *DataStatus::GetDataMgr(string name) const
DataMgr *DataStatus::GetDataMgr(string name) const {

	if (name.empty()) name = "DataSet1";

	map <string, DataMgr *>::const_iterator itr;
	itr = _dataMgrs.find(name);

	if (itr == _dataMgrs.end()) return(NULL); 

	return(itr->second);
}

vector <string> DataStatus::GetDataMgrNames() const {
	vector <string> names;

	map <string, DataMgr *>::const_iterator itr;
	for (itr = _dataMgrs.begin(); itr != _dataMgrs.end(); ++itr) {
		names.push_back(itr->first);
	}
	return(names);
}

void DataStatus::GetExtents(
	size_t ts, 
	const map <string, vector <string>> &varMap,
	vector <double> &minExts, vector <double> &maxExts
) const {
	minExts.resize(3, 0.0);
	maxExts.resize(3, 1.0);

	if (varMap.empty()) return;

    vector <double> tmpMinExts(3, std::numeric_limits<double>::max());
    vector <double> tmpMaxExts(3, std::numeric_limits<double>::lowest());

	map <string, vector <string>>::const_iterator itr;
	for (itr = varMap.begin(); itr != varMap.end(); ++itr) {
		string dataSetName = itr->first;

		DataMgr *dataMgr = GetDataMgr(dataSetName);
		if (! dataMgr) continue;

		size_t local_ts = MapGlobalToLocalTimeStep(dataSetName, ts);

		const vector <string> &varnames = itr->second;

		vector <double> minVExts, maxVExts;
		vector <int> axes;
		bool status = DataMgrUtils::GetExtents(
			dataMgr, local_ts, varnames, minVExts, maxVExts, axes
		);
		if (! status) continue;

		for (int i=0; i<axes.size(); i++) {
			int axis = axes[i];

			if (minVExts[i] < tmpMinExts[axis]) {
				tmpMinExts[axis] = minVExts[i];
			}
			if (maxVExts[i] > tmpMaxExts[axis]) {
				tmpMaxExts[axis] = maxVExts[i];
			}
		}
	}

	// tmp{Min,Max}Exts are always 3D vectors. If all variables are 2D
	// and live in same plane the returned {min,max}Exts should have
	// size of 2.
	//
	for (int i=0; i<tmpMinExts.size(); i++) {
		if (tmpMinExts[i] != std::numeric_limits<double>::max()) {
			minExts[i] = tmpMinExts[i];
			maxExts[i] = tmpMaxExts[i];
		}
	}
    return;
}

map <string, vector <string>> DataStatus::getFirstVars(
	const vector <string> &dataSetNames
) const {
	map <string, vector <string>> defaultVars;

	for (int i=0; i<dataSetNames.size(); i++) {
		DataMgr *dataMgr = GetDataMgr(dataSetNames[i]);
		vector <string> varnames;
		for (int dim=3; dim>1; dim--) {
			varnames = dataMgr->GetDataVarNames(dim, true);
			if (varnames.size()) {
				vector <string> oneVar(1, varnames[0]);
				defaultVars[dataSetNames[i]] = oneVar;
				break;
			}
		}
	}
	return(defaultVars);
}

void DataStatus::GetActiveExtents(
	const ParamsMgr *paramsMgr, string winName, size_t ts,
	vector <double> &minExts, vector <double> &maxExts
) const {

	vector <string> dataSetNames = GetDataMgrNames();
	
	map <string, vector <string>> varMap;

	bool foundOne = false;
	for (int i=0; i<dataSetNames.size(); i++) {

		vector <RenderParams *> rParams;
		paramsMgr->GetRenderParams(winName, dataSetNames[i], rParams);

		vector <string> varnames;
		for (int j=0; j<rParams.size(); j++) {
			if (! rParams[j]->IsEnabled()) continue;
			string varname = rParams[j]->GetVariableName();
			if (! varname.empty()) {
				varnames.push_back(varname);
			}

			vector <string> fvarnames = rParams[j]->GetFieldVariableNames();
			for (int k=0; k<fvarnames.size(); k++) {
				if (! fvarnames[k].empty()) {
					varnames.push_back(fvarnames[k]);
				}
			}
		}
		if (varnames.size()) {
			foundOne = true;
		}
		varMap[dataSetNames[i]] = varnames;
	}

	// If we didn't find any enabled variable use the first variables
	// found in each data set
	//
	if (! foundOne) {
		varMap = getFirstVars(dataSetNames);
	}

	GetExtents(ts, varMap, minExts, maxExts);
}

void DataStatus::GetActiveExtents(
	const ParamsMgr *paramsMgr, size_t ts,
	vector <double> &minExts, vector <double> &maxExts
) const {

    minExts.resize(3, std::numeric_limits<double>::max());
    maxExts.resize(3, std::numeric_limits<double>::lowest());

	vector <string> winNames = paramsMgr->GetVisualizerNames();

	for (int i=0; i<winNames.size(); i++) {
		vector <double> minWExts, maxWExts;
		GetActiveExtents(paramsMgr, winNames[i], ts, minWExts, maxWExts);

		for (int j=0; j<minWExts.size(); j++) {
			if (minWExts[j] < minExts[j]) {
				minExts[j] = minWExts[j];
			}
			if (maxWExts[j] > maxExts[j]) {
				maxExts[j] = maxWExts[j];
			}
		}
	}
}

size_t DataStatus::MapGlobalToLocalTimeStep(
	string dataSetName, size_t ts
) const {

	map <string, vector <size_t>>::const_iterator itr;
	itr = _timeMap.find(dataSetName);
	if (itr == _timeMap.end()) return(0);

	const vector <size_t> &ref = itr->second;
	if (ts >= ref.size()) {
		ts = ref.size()-1;
	}
	return(ref[ts]);
}

void DataStatus::MapLocalToGlobalTimeRange(
	string dataSetName, size_t local_ts, size_t &min_ts, size_t &max_ts
) const {
	min_ts = max_ts = 0;

	map <string, vector <size_t>>::const_iterator itr;
	itr = _timeMap.find(dataSetName);
	if (itr == _timeMap.end()) return;

	const vector <size_t> &ref = itr->second;
	vector <size_t>::const_iterator itr1;
	vector <size_t>::const_reverse_iterator itr2;

	itr1 = find(ref.begin(), ref.end(), local_ts);
	if (itr1 == ref.end()) return;

	itr2 = find(ref.rbegin(), ref.rend(), local_ts);
	if (itr2 == ref.rend()) return;

	min_ts = itr1 - ref.begin();
	max_ts = ref.rend() - itr2 - 1;
}

namespace {
int find_nearest(const vector <double> &timeCoords, double time) {
	if (! timeCoords.size()) return(-1);
	
	if (time <= timeCoords[0]) return (0);
	if (time >= timeCoords[timeCoords.size()-1]) return (timeCoords.size()-1);

	// If we get to here there must be at least two elements in timeCoords
	//
	assert(timeCoords.size() >= 2);

	for (int i=0; i<timeCoords.size() - 1; i++) {
		if (time >= timeCoords[i] && time <= timeCoords[i+1]) {
			assert(timeCoords[i] != timeCoords[i+1]);

			double s = (time-timeCoords[i]) / (timeCoords[i+1]-timeCoords[i]);
			if (s <= 0.5) {
				return (i);
			}
			else {
				return (i+1);
			}
		}
	}
	assert(0);
}
};

void DataStatus::reset_time() {
	_timeCoords.clear();
	_timeMap.clear();

	if (_dataMgrs.size() == 0) return;

	// First construct global list of time coordinates
	//
	map <string, DataMgr *>::const_iterator itr;
	for (itr = _dataMgrs.begin(); itr != _dataMgrs.end(); ++itr) {
		DataMgr *dataMgr = itr->second;

		vector <double> t = dataMgr->GetTimeCoordinates();
		_timeCoords.insert(_timeCoords.end(), t.begin(), t.end());
	}

	sort(_timeCoords.begin(), _timeCoords.end());
	unique(_timeCoords.begin(), _timeCoords.end());

	for (itr = _dataMgrs.begin(); itr != _dataMgrs.end(); ++itr) {
		string dataSetName = itr->first;
		DataMgr *dataMgr = itr->second;

		vector <size_t> timeSteps;
		vector <double> t = dataMgr->GetTimeCoordinates();
		for (int i=0; i<_timeCoords.size(); i++) {
			int idx = find_nearest(t, _timeCoords[i]);
			timeSteps.push_back(idx);
		}
		_timeMap[dataSetName] = timeSteps;
	}
}

DataStatus::~DataStatus(){
	
}




#ifdef	DEAD
//Map corners of box to voxels.  
void DataStatus::mapBoxToVox(
	Box* box, string varname, int refLevel, int lod, int timestep, 
	size_t voxExts[6]
) {
	double userExts[6];
	box->GetUserExtents(userExts,(size_t)timestep);
	vector<double>minexts, maxexts;
	for (int i = 0; i<3; i++){
		minexts.push_back( userExts[i]);
		maxexts.push_back( userExts[i+3]);
	}
	bool errEnabled = MyBase::EnableErrMsg(false);
	StructuredGrid* rg = dataMgr->GetVariable(timestep,varname,refLevel,lod,minexts, maxexts);
	MyBase::EnableErrMsg(errEnabled);
	
	if (rg){
		rg->GetIJKIndex(minexts[0],minexts[1],minexts[2],voxExts, voxExts+1, voxExts+2);
		rg->GetIJKIndex(maxexts[0],maxexts[1],maxexts[2],voxExts+3, voxExts+4, voxExts+5);
	} else {
		for (int i = 0; i<6; i++) voxExts[i] = 0;
	}
	//(Note: this can be expensive with layered data)
	return;

}

#endif
