//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		regionparams.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		September 2004
//
//	Description:	Implements the RegionParams class.
//		This class supports parameters associted with the
//		region panel
//
#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning(disable : 4100)
#endif

#include <iostream>

#include <vapor/regionparams.h>

using namespace VAPoR;

const string RegionParams::_domainVariablesTag = "DomainVariables";

//
// Register class with object factory!!!
//
static ParamsRegistrar<RegionParams> registrar(RegionParams::GetClassType());

RegionParams::RegionParams(
    ParamsBase::StateSave *ssave) : ParamsBase(ssave, RegionParams::GetClassType()) {

    // Initialize DataMgr dependent parameters
    //
    _init();

    m_Box = new Box(ssave);
    m_Box->SetParent(this);
}

RegionParams::RegionParams(
    ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node) {

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != RegionParams::GetClassType()) {
        node->SetTag(RegionParams::GetClassType());
        _init();
    }

    if (node->HasChild(Box::GetClassType())) {
        m_Box = new Box(ssave, node->GetChild(Box::GetClassType()));
    } else {

        // Node doesn't contain a Box
        //
        m_Box = new Box(ssave);
        m_Box->SetParent(this);
    }

    _reconcile();
}

RegionParams::RegionParams(const RegionParams &rhs) : ParamsBase(rhs) {

    m_Box = new Box(*(rhs.m_Box));
}

RegionParams &RegionParams::operator=(const RegionParams &rhs) {

    m_Box = new Box(*(rhs.m_Box));

    return (*this);
}

RegionParams::~RegionParams() {
    if (m_Box)
        delete m_Box;
}

//Reset region settings to initial state
void RegionParams::_init() {

#ifdef DEAD
    const double *extents = m_dataStatus->getLocalExtents();
    vector<double> exts(3, 0.0);

    for (i = 0; i < 3; i++) {
        exts.push_back(extents[i + 3] - extents[i]);
    }
    GetBox()->SetLocalExtents(exts);
    GetBox()->Trim();

    vector<string> novars;
    SetDomainVariables(novars);
#endif
}

void RegionParams::_reconcile() {

#ifdef DEAD
    vector<long> times = GetBox()->GetTimes();
    double regionExtents[6];
    for (int timenum = 0; timenum < times.size(); timenum++) {
        int currTime = times[timenum];
        GetBox()->GetLocalExtents(regionExtents, currTime);

        //force them to fit in current volume
        for (i = 0; i < 3; i++) {

            if (regionExtents[i] > extents[i + 3] - extents[i])
                regionExtents[i] = extents[i + 3] - extents[i];
            if (regionExtents[i] < 0.)
                regionExtents[i] = 0.;
            if (regionExtents[i + 3] > extents[i + 3] - extents[i])
                regionExtents[i + 3] = extents[i + 3] - extents[i];
            if (regionExtents[i + 3] < 0.)
                regionExtents[i + 3] = 0.;
            if (regionExtents[i] > regionExtents[i + 3])
                regionExtents[i + 3] = regionExtents[i];
        }
        exts.clear();
        for (int j = 0; j < 6; j++)
            exts.push_back(regionExtents[j]);
        GetBox()->SetLocalExtents(exts, this, currTime);
    }
#endif
}

#ifdef DEAD
bool RegionParams::insertTime(int timestep) {

    return true;
}

bool RegionParams::removeTime(int timestep) {
    return true;
}
#endif
