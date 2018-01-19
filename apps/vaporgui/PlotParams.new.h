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

namespace VAPoR{

class PlotParams : public RenderParams 
{
public:
    PlotParams(DataMgr* dmgr, ParamsBase::StateSave *ssave);
    PlotParams(DataMgr* dmgr, ParamsBase::StateSave *ssave, XmlNode *node);
    ~PlotParams();


    int  GetMinTS() const;
    void SetMinTS(int ts);

    int  GetMaxTS() const;
    void SetMaxTS(int ts);

    int  GetOneTS() const;
    void SetOneTS( int ts );

    bool GetSpaceTimeMode() const;
    void SetSpaceMode();
    void SetTimeMode();
    

    static string GetClassType() 
    {
        return("StatisticsParams");
    }

    // virtual functions required by RenderParams
    virtual bool IsOpaque() const
    {
        return true; 
    }
    virtual bool usingVariable( const std::string& varname )
    {
        return false;
    }

private:
    static const string _minTSTag;
    static const string _maxTSTag;
    static const string _oneTSTag;
    static const string _spaceTimeTag;  // Space=true, Time=false
};

}; // End namespace VAPoR
#endif 
