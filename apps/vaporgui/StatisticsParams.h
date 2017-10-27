//************************************************************************
//                                  *
//           Copyright (C)  2004                *
//     University Corporation for Atmospheric Research          *
//           All Rights Reserved                *
//                                  *
//************************************************************************/
//
//  File:       StatisticsParams.h
//
//  Author:     Scott Pearse
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:       August 2017
//
//  Description:    Defines the StatisticsParams class.
//

#ifndef STATISTICSPARAMS_H
#define STATISTICSPARAMS_H

#include <vapor/RenderParams.h>

namespace VAPoR {

class StatisticsParams : public RenderParams {
public:
    StatisticsParams(DataMgr *dmgr, ParamsBase::StateSave *ssave);
    StatisticsParams(DataMgr *dmgr, ParamsBase::StateSave *ssave, XmlNode *node);
    ~StatisticsParams();

    bool GetAutoUpdate();
    void SetAutoUpdate(bool state);

    // int GetRegionSelection();
    // void SetRegionSelection(int state);

    int  GetMinTS();
    void SetMinTS(int ts);

    int  GetMaxTS();
    void SetMaxTS(int ts);

    // vector<double> GetMinExtents();
    // void SetMinExtents(vector<double> minExts);
    // vector<double> GetMaxExtents();
    // void SetMaxExtents(vector<double> maxExts);

    // int GetCRatio();
    // void SetCRatio(int cRatio);
    // int GetRefinement();
    // void SetRefinement(int ref);

    bool GetMinEnabled();
    void SetMinEnabled(bool state);

    bool GetMaxEnabled();
    void SetMaxEnabled(bool state);

    bool GetMeanEnabled();
    void SetMeanEnabled(bool state);

    bool GetMedianEnabled();
    void SetMedianEnabled(bool state);

    bool GetStdDevEnabled();
    void SetStdDevEnabled(bool state);

    // should be using RenderParams::GetAuxVariableNames()
    // vector<string> GetVarNames();
    // void SetVarNames(vector<string> varNames);

    // Get static string id for this params class
    //
    static string GetClassType() { return ("StatisticsParams"); }

    // virtual functions required by RenderParams
    virtual bool IsOpaque() const { return true; }
    virtual bool usingVariable(const std::string &varname) { return false; }

private:
    // static const string _varsTag;
    // static const string _vars3dTag;
    // static const string _statisticsTag;
    // static const string _dataSourceTag;
    // static const string _refinementTag;
    // static const string _cRatioTag;
    static const string _minTSTag;
    static const string _maxTSTag;
    static const string _autoUpdateTag;
    // static const string _minExtentsTag;
    // static const string _maxExtentsTag;
    // static const string _regionSelectTag;
    static const string _minEnabledTag;
    static const string _maxEnabledTag;
    static const string _meanEnabledTag;
    static const string _medianEnabledTag;
    static const string _stdDevEnabledTag;
};

};        // End namespace VAPoR
#endif    // STATISTICSPARAMS_H
