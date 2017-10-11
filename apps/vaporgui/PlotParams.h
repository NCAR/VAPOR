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

namespace VAPoR{

class PlotParams : public ParamsBase {
 public:
  PlotParams(ParamsBase::StateSave *ssave);
  PlotParams(ParamsBase::StateSave *ssave, XmlNode *node);
  ~PlotParams();

  int GetRegionSelection() const;
  void SetRegionSelection(int state);

  //int GetSpaceMinTS() const;
  //void SetSpaceMinTS(int ts);

  int GetMaxTS() const;
  void SetMaxTS(int ts);

  vector<double> GetSpaceMinExtents() const;
  void SetSpaceMinExtents(vector<double> minExts);

  vector<double> GetSpaceMaxExtents() const;
  void SetSpaceMaxExtents(vector<double> maxExts);

  vector<double> GetTimePoint() const;
  void SetTimePoint(vector<double> maxExts);

  int GetCRatio() const;
  void SetCRatio(int cRatio);

  int GetRefinement() const;
  void SetRefinement(int ref);

  vector<string> GetVarNames() const;
  void SetVarNames(vector<string> varNames);

  string GetSpaceOrTime() const;
  void SetSpaceOrTime(string state);

  int GetSpaceTS() const;
  void SetSpaceTS(int time);

  int GetTimeMinTS() const;
  void SetTimeMinTS(int time);

  int GetTimeMaxTS() const;
  void SetTimeMaxTS(int time);

  double GetTimeXCoord() const;
  void SetTimeXCoord(double coord);

  double GetTimeYCoord() const;
  void SetTimeYCoord(double coord);

  double GetTimeZCoord() const;
  void SetTimeZCoord(double coord);

  bool GetXConst() const;
  void SetXConst(bool state);

  bool GetYConst() const;
  void SetYConst(bool state);

  bool GetZConst() const;
  void SetZConst(bool state);

  bool GetTimeConst() const;
  void SetTimeConst(bool state);

  // Get static string id for this params class
  //
  static string GetClassType() {
	return("PlotParams");
  }

 private:
  static const string _varsTag;
  static const string _vars3dTag;
  static const string _dataSourceTag;
  static const string _refinementTag;
  static const string _cRatioTag;
  static const string _spaceMinTSTag;
  static const string _spaceMaxTSTag;
  static const string _spaceMinExtentsTag;
  static const string _spaceMaxExtentsTag;
  static const string _spaceOrTimeTag;
  static const string _timePointTag;
  static const string _timeMinTSTag;
  static const string _timeMaxTSTag;
  static const string _timeXTag;
  static const string _timeYTag;
  static const string _timeZTag;
  static const string _xConstTag;
  static const string _yConstTag;
  static const string _zConstTag;
  static const string _timeConstTag;
};

}; // End namespace VAPoR
#endif // PLOTPARAMS_H
