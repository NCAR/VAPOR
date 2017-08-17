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

#include <vapor/ParamsBase.h>

namespace VAPoR{

class PARAMS_API StatisticsParams : public ParamsBase {
 public:
  StatisticsParams(ParamsBase::StateSave *ssave);
  StatisticsParams(ParamsBase::StateSave *ssave, XmlNode *node);
  ~StatisticsParams();

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

  bool GetMinStat();
  void SetMinStat(bool state);

  bool GetMaxStat();
  void SetMaxStat(bool state);

  bool GetMeanStat();
  void SetMeanStat(bool state);

  bool GetMedianStat();
  void SetMedianStat(bool state);

  bool GetStdDevStat();
  void SetStdDevStat(bool state);

  vector<string> GetVarNames();
  void SetVarNames(vector<string> varNames);

  // Get static string id for this params class
  //
  static string GetClassType() {
	return("StatisticsParams");
  }

 private:
  static const string _varsTag;
  static const string _vars3dTag;
  static const string _statisticsTag;
  static const string _dataSourceTag;
  static const string _refinementTag;
  static const string _cRatioTag;
  static const string _minTSTag;
  static const string _maxTSTag;
  static const string _autoUpdateTag;
  static const string _minExtentsTag;
  static const string _maxExtentsTag;
  static const string _regionSelectTag;
  static const string _minStatTag;
  static const string _maxStatTag;
  static const string _meanStatTag;
  static const string _medianStatTag;
  static const string _stdDevStatTag;
};

}; // End namespace VAPoR
#endif // STATISTICSPARAMS_H
