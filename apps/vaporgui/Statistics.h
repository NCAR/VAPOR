//************************************************************************
//                                                                    *
//         Copyright (C)  2016                                      *
//   University Corporation for Atmospheric Research                  *
//         All Rights Reserved                                      *
//                                                                    *
//************************************************************************/
//
//  File:      Statistics.h
//
//  Author:  Scott Pearse
//        National Center for Atmospheric Research
//        PO 3000, Boulder, Colorado
//
//  Date:      August 2016
//
//  Description:    Implements the Statistics class.
//
#ifdef WIN32
#pragma warning(disable : 4100)
#endif

#ifndef STATISTICS_H
#define STATISTICS_H

#include <qdialog.h>
#include <qwidget.h>
#include <vapor/DataMgr.h>
#include <vapor/Grid.h>
#include <vapor/ControlExecutive.h>
#include "ui_statsWindow.h"
#include "ui_errMsg.h"
#include "RangeController.h"
#include <RangeCombos.h>
#include <StatisticsParams.h>

//
//! \class Statistics
//! \brief ???
//!
//! \author Scott Pearse
//! \version $revision$
//! \date $Date$
//!
//!

namespace VAPoR {
class RenderParams;
class ParamsMgr;
class DataMgr;
} // namespace VAPoR

class sErrMsg : public QDialog, public Ui_ErrMsg {
    Q_OBJECT
  public:
    sErrMsg() { setupUi(this); }
};

class Statistics : public QDialog, public Ui_StatsWindow {

    Q_OBJECT

  public:
    Statistics(QWidget *parent);
    ~Statistics();
    int initControlExec(VAPoR::ControlExec *ce);
    int initDataMgr(VAPoR::DataMgr *dm);
    void showMe();
    int initialize();
    void Update(VAPoR::StatisticsParams *sParams);

  private slots:
    void restoreExtents();
    void minTSChanged();
    void maxTSChanged();
    void autoUpdateClicked();
    void refinementChanged(int);
    void cRatioChanged(int);
    void newVarAdded(int);
    void updateButtonPressed() {
        updateStats();
    }
    void initRegion();
    void varRemoved(int);
    void exportText();
    void rangeComboChanged();
    void addStatistic(int);
    void removeStatistic(int);

  private:
    VAPoR::StatisticsParams *_params;
    void retrieveRangeParams();
    bool eventFilter(QObject *o, QEvent *e);
    int GetExtents(vector<double> &extents);
    int initVariables();
    void adjustTables(); // ???
    void initCRatios();
    void initRefinement();
    void initTimes();
    // Need replacement
    void initRanges();
    void setNewExtents();
    void updateStats();
    void refreshTable();
    void generateTableColumns();
    void addCalculationToTable(string varname);
    void updateRangeCombo();
    void makeItRed();
    void errReport(string msg) const;
    void rGridError(int ts, string varname);
    void updateVariables();
    void updateStatisticSelection();

    bool calcMinMax(string varname);
    bool calcMean(string varname);
    bool calcMedian(string varname);
    bool calcStdDev(string varname);
    void getMultiPointTSMean(double &tsMean, int &missing, int &count, VAPoR::Grid *rGrid);

    struct _statistics {
        size_t row;
        double min;
        double max;
        double mean;
        double median;
        double stddev;
    };

    unsigned char _MIN;
    unsigned char _MAX;
    unsigned char _MEAN;
    unsigned char _SIGMA;
    unsigned char _MEDIAN;
    unsigned char _calculations;

    sErrMsg *_errMsg;

    Range *_xRange;
    Range *_yRange;
    Range *_zRange;

    VAPoR::ControlExec *_controlExec;
    VAPoR::DataStatus *_dataStatus;
    VAPoR::DataMgr *_dm;
    VAPoR::Grid *_rGrid;
    string _defaultVar;
    vector<string> _vars;
    vector<string> _vars3d;
    vector<size_t> _cRatios;
    vector<double> _extents;
    vector<double> _fullExtents;
    vector<double> _uCoordMin;
    vector<double> _uCoordMax;
    size_t _vCoordMin[3];
    size_t _vCoordMax[3];
    map<string, _statistics> _stats;
    size_t _minTS;   // what's the difference of these 2 pairs?
    size_t _maxTS;   //
    size_t _minTime; // what's the difference of these 2 pairs?
    size_t _maxTime; //
    int _refLevel;   // what's the difference of these 2 variables?
    int _refLevels;  //
    int _regionSelection;
    size_t _cRatio;
    bool _autoUpdate;
    bool _regionInitialized;
    bool _initialized;
    bool _rangeComboInitialized;
};
#endif
