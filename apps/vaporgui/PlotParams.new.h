//************************************************************************
//                                                                       *
//           Copyright (C)  2004                                         *
//     University Corporation for Atmospheric Research                   *
//           All Rights Reserved                                         *
//                                                                       *
//************************************************************************
//
//  File:       PlotParams.h
//
//  Author:     Samuel Li
//              National Center for Atmospheric Research
//              PO 3000, Boulder, Colorado
//
//  Date:       December 2017
//
//  Description:    Defines the PlotParams class.
//

#ifndef PLOTPARAMS_H
#define PLOTPARAMS_H

#include <vapor/RenderParams.h>

namespace VAPoR {

/// PlotParams inherits RenderParams.
class PlotParams : public RenderParams {
public:
    /// Constructor 1
    PlotParams(DataMgr *dmgr, ParamsBase::StateSave *ssave);
    /// Constructor 2
    PlotParams(DataMgr *dmgr, ParamsBase::StateSave *ssave, XmlNode *node);
    /// Destructor
    ~PlotParams();

    /// In ``time mode,'' these 4 methods get/set the time range.
    int  GetMinTS() const;
    void SetMinTS(int ts);
    int  GetMaxTS() const;
    void SetMaxTS(int ts);

    /// Get/set the current operational mode: space or time.
    bool GetSpaceTimeMode() const;
    void SetSpaceMode();
    void SetTimeMode();

    static string GetClassType() { return ("PlotParams"); }

    // virtual functions required by RenderParams
    virtual bool IsOpaque() const { return true; }
    virtual bool usingVariable(const std::string &varname) { return false; }

private:
    static const string _minTSTag;
    static const string _maxTSTag;
    static const string _oneTSTag;
    static const string _spaceTimeTag;    // Space=true, Time=false
};

};    // End namespace VAPoR
#endif
