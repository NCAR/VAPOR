//************************************************************************
//																	  *
//		   Copyright (C)  2016										*
//	 University Corporation for Atmospheric Research				  *
//		   All Rights Reserved										*
//																	  *
//************************************************************************/
//
//  File:	   Statistics.h
//
//  Author:	 Scott Pearse
//		  National Center for Atmospheric Research
//		  PO 3000, Boulder, Colorado
//
//  Date:	   August 2016
//
//  Description:	Implements the Statistics class.
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

  private slots:
    void restoreExtents();
    void minTSChanged();
    void maxTSChanged();
    void autoUpdateClicked();
    void refinementChanged(int);
    void cRatioChanged(int);
    void newVarAdded(int);
    void updateButtonPressed() { update(); }
    void initRegion();
    void copyActiveRegion();
    void varRemoved(int);
    void exportText();
    void regionSlidersChanged();
    void addStatistic(int);
    void removeStatistic(int);

  private:
    VAPoR::StatisticsParams *_params;
    void retrieveRangeParams();
    bool eventFilter(QObject *o, QEvent *e);
    int GetExtents(vector<double> &extents);
    int initVariables();
    void adjustTables();
    void initCRatios();
    void initRefinement();
    void initTimes();
    void initRangeControllers();
    void setNewExtents();
    void update();
    void refreshTable();
    void generateTableColumns();
    void addCalculationToTable(string varname);
    void makeItRed();
    void updateSliders();
    void errReport(string msg) const;
    void rGridError(int ts, string varname);

    bool calcMinMax(string varname);
    bool calcMean(string varname);
    bool calcMedian(string varname);
    bool calcStdDev(string varname);
    void getSinglePointTSMean(double &tsMean,
                              int &missing, VAPoR::Grid *rGrid);
    void getMultiPointTSMean(double &tsMean,
                             int &missing, int &count, VAPoR::Grid *rGrid);
    void getSinglePointTSStdDev(double &tsStdDev,
                                int &globalCount, int &spMissing, double mean,
                                VAPoR::Grid *rGrid);

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
    MinMaxSlider *_xMinSlider;
    MinMaxSlider *_xMaxSlider;
    MinMaxLineEdit *_xMinLineEdit;
    MinMaxLineEdit *_xMaxLineEdit;
    SinglePointSlider *_xSinglePointSlider;
    SinglePointLineEdit *_xSinglePointLineEdit;
    CenterSizeSlider *_xCenterSlider;
    CenterSizeSlider *_xSizeSlider;
    CenterSizeLineEdit *_xCenterLineEdit;
    CenterSizeLineEdit *_xSizeLineEdit;
    MinMaxTableCell *_xMinCell;
    MinMaxTableCell *_xMaxCell;
    MinMaxLabel *_minXMinLabel;
    MinMaxLabel *_minXMaxLabel;
    MinMaxLabel *_maxXMinLabel;
    MinMaxLabel *_maxXMaxLabel;
    MinMaxLabel *_centerXMinLabel;
    MinMaxLabel *_centerXMaxLabel;
    SizeLabel *_sizeXMinLabel;
    SizeLabel *_sizeXMaxLabel;
    MinMaxLabel *_spXMinLabel;
    MinMaxLabel *_spXMaxLabel;

    Range *_yRange;
    MinMaxSlider *_yMinSlider;
    MinMaxSlider *_yMaxSlider;
    MinMaxLineEdit *_yMinLineEdit;
    MinMaxLineEdit *_yMaxLineEdit;
    SinglePointSlider *_ySinglePointSlider;
    SinglePointLineEdit *_ySinglePointLineEdit;
    CenterSizeSlider *_yCenterSlider;
    CenterSizeSlider *_ySizeSlider;
    CenterSizeLineEdit *_yCenterLineEdit;
    CenterSizeLineEdit *_ySizeLineEdit;
    MinMaxTableCell *_yMinCell;
    MinMaxTableCell *_yMaxCell;
    MinMaxLabel *_minYMinLabel;
    MinMaxLabel *_minYMaxLabel;
    MinMaxLabel *_maxYMinLabel;
    MinMaxLabel *_maxYMaxLabel;
    MinMaxLabel *_centerYMinLabel;
    MinMaxLabel *_centerYMaxLabel;
    SizeLabel *_sizeYMinLabel;
    SizeLabel *_sizeYMaxLabel;
    MinMaxLabel *_spYMinLabel;
    MinMaxLabel *_spYMaxLabel;

    Range *_zRange;
    MinMaxSlider *_zMinSlider;
    MinMaxSlider *_zMaxSlider;
    MinMaxLineEdit *_zMinLineEdit;
    MinMaxLineEdit *_zMaxLineEdit;
    SinglePointSlider *_zSinglePointSlider;
    SinglePointLineEdit *_zSinglePointLineEdit;
    CenterSizeSlider *_zCenterSlider;
    CenterSizeSlider *_zSizeSlider;
    CenterSizeLineEdit *_zCenterLineEdit;
    CenterSizeLineEdit *_zSizeLineEdit;
    MinMaxTableCell *_zMinCell;
    MinMaxTableCell *_zMaxCell;
    MinMaxLabel *_minZMinLabel;
    MinMaxLabel *_minZMaxLabel;
    MinMaxLabel *_maxZMinLabel;
    MinMaxLabel *_maxZMaxLabel;
    MinMaxLabel *_centerZMinLabel;
    MinMaxLabel *_centerZMaxLabel;
    SizeLabel *_sizeZMinLabel;
    SizeLabel *_sizeZMaxLabel;
    MinMaxLabel *_spZMinLabel;
    MinMaxLabel *_spZMaxLabel;

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
    map<string, _statistics> _stats;
    size_t _minTS;
    size_t _maxTS;
    int _refLevel;
    int _regionSelection;
    size_t _cRatio;
    size_t _vCoordMin[3];
    size_t _vCoordMax[3];
    size_t _minTime;
    size_t _maxTime;
    bool _autoUpdate;
    bool _regionInitialized;
    bool _initialized;
    bool _slidersInitialized;
};
#endif
