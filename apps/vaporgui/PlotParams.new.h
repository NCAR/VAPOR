//************************************************************************
//                                                                       *
//           Copyright (C)  2004                                         *
//     University Corporation for Atmospheric Research                   *
//           All Rights Reserved                                         *
//                                                                       *
//************************************************************************
//
//  File:       StatisticsParams.h
//
//  Author:     Samuel Li
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:       December 2017
//
//  Description:    Defines the PlotParams class.
//

#ifndef PLOTPARAMS_H
#define PLOTPARAMS_H

#include <vapor/RenderParams.h>

namespace VAPoR {

class PlotParams : public RenderParams {
public:
    PlotParams(DataMgr *dmgr, ParamsBase::StateSave *ssave);
    PlotParams(DataMgr *dmgr, ParamsBase::StateSave *ssave, XmlNode *node);
    ~PlotParams();

    bool GetAutoUpdateEnabled();
    void SetAutoUpdateEnabled(bool state);

    int  GetCurrentMinTS() const;
    void SetCurrentMinTS(int ts);

    int  GetCurrentMaxTS() const;
    void SetCurrentMaxTS(int ts);

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

    static string GetClassType() { return ("StatisticsParams"); }

    // virtual functions required by RenderParams
    virtual bool IsOpaque() const { return true; }
    virtual bool usingVariable(const std::string &varname) { return false; }

private:
    static const string _minTSTag;
    static const string _maxTSTag;
    static const string _autoUpdateTag;
    static const string _minEnabledTag;
    static const string _maxEnabledTag;
    static const string _meanEnabledTag;
    static const string _medianEnabledTag;
    static const string _stdDevEnabledTag;
};

};    // End namespace VAPoR
#endif
