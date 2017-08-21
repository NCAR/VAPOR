//************************************************************************
//                                  *
//           Copyright (C)  2004                *
//     University Corporation for Atmospheric Research          *
//           All Rights Reserved                *
//                                  *
//************************************************************************/
//
//  File:       PlotParams.h
//
//  Author:     Scott Pearse
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:       August 2017
//
//  Description:    Defines the PlotParams class.
//

#ifndef PLOTPARAMS_H
#define PLOTPARAMS_H

#include <vapor/ParamsBase.h>

namespace VAPoR {

class PARAMS_API PlotParams : public ParamsBase {
  public:
    PlotParams(ParamsBase::StateSave *ssave);
    PlotParams(ParamsBase::StateSave *ssave, XmlNode *node);
    ~PlotParams();

    bool GetAutoUpdate();
    void SetAutoUpdate(bool state);

    int GetRegionSelection();
    void SetRegionSelection(int state);

    int GetMinTS();
    void SetMinTS(int ts);

    int GetMaxTS();
    void SetMaxTS(int ts);

    vector<double> GetMinExtents();
    void SetMinExtents(vector<double> minExts);

    vector<double> GetMaxExtents();
    void SetMaxExtents(vector<double> maxExts);

    int GetCRatio();
    void SetCRatio(int cRatio);

    int GetRefinement();
    void SetRefinement(int ref);

    vector<string> GetVarNames();
    void SetVarNames(vector<string> varNames);

    string GetSpaceOrTime();
    void SetSpaceOrTime(string state);

    // Get static string id for this params class
    //
    static string GetClassType() {
        return ("PlotParams");
    }

  private:
    static const string _varsTag;
    static const string _vars3dTag;
    static const string _dataSourceTag;
    static const string _refinementTag;
    static const string _cRatioTag;
    static const string _minTSTag;
    static const string _maxTSTag;
    static const string _minExtentsTag;
    static const string _maxExtentsTag;
    static const string _spaceOrTimeTag;
};

};     // End namespace VAPoR
#endif // PLOTPARAMS_H
