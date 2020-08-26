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
//  Date:       November 2017
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

    bool GetAutoUpdateEnabled();
    void SetAutoUpdateEnabled(bool state);

    // Note: we'll use the Get/SetCurrentTimestep() from RendererParams to
    // represent the min timestep, MinTS, so we only need to keep track of MaxTS.
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

    static string  GetClassType() { return ("StatisticsParams"); }
    virtual size_t GetRenderDim() const override { return (0); }

private:
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
