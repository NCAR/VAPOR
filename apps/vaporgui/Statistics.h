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

namespace VAPoR {
    class ParamsMgr;
    class DataMgr;
}

class sErrMsg : public QDialog, public Ui_ErrMsg 
{
    Q_OBJECT

public:
    sErrMsg() 
    {
        setupUi(this);
    }
};

class Statistics : public QDialog, public Ui_StatsWindow 
{
    Q_OBJECT

public:
    Statistics(QWidget* parent);
    ~Statistics();  
    int initControlExec(VAPoR::ControlExec* ce);
    void showMe();
    bool Update();


protected:
    // Keeps the current variables shown and their statistical values.
    // Invalid values are stored as std::nan("1");
    // 
    class ValidStats
    {
    public:
        bool AddVariable( std::string& );
        bool RemoveVariable( std::string& );
        size_t GetVariableCount();
        std::string GetVariableName( int i );
        
        bool Add3MStats( std::string&, const double* );   // Min, Max, Mean
        bool AddMedian(  std::string&, double );
        bool AddStddev(  std::string&, double );
        bool AddCount(   std::string&, long );

        // invalid values are represented as nan.
        bool Get3MStats( std::string&, double* );
        bool GetMedian(  std::string&, double* );
        bool GetStddev(  std::string&, double* );
        bool GetCount(   std::string&, long* );

        bool InvalidAll();

        std::string GetDatasetName();
        bool SetDatasetName( std::string& );
        
    private:
        std::vector<std::string>    _variables;
        std::vector<double>         _values[5];  // 0: min
                                                // 1: max
                                                // 2: mean
                                                // 3: median
                                                // 4: stddev
        std::vector<long>           _count;     // number of samples
        int _getVarIdx( std::string& );          // -1: not exist
                                                // >=0: a valid index
        std::string _datasetName;
    };  // finish class ValidStats

    bool Connect();   // connect slots 

private slots:
    void _newVarChanged( int );
    void _removeVarChanged( int );
    void _newCalcChanged( int );
    void _removeCalcChanged( int );
    void _refinementChanged( int );
    void _lodChanged( int );
    void _minTSChanged( int );
    void _maxTSChanged( int );
    void _updateButtonClicked();
    void _geometryValueChanged();
    void _dataSourceChanged( int );
    /*
    void restoreExtents();
    void minTSChanged();
    void maxTSChanged();
    void autoUpdateClicked();
    void refinementChanged(int);
    void cRatioChanged(int);
    void addVariable(int);
    void removeVariable(int);
    void initRegion();
    void exportText();
    void rangeComboChanged();
    void addStatistic(int);
    void removeStatistic(int);
    void updateButtonPressed() 
    {
        updateStats();
    }
    */


private:
    ValidStats          _validStats;
    sErrMsg*            _errMsg;
    VAPoR::ControlExec* _controlExec;

    void                _updateStatsTable();

    // calculations should put results in _validStats directly.
    bool                _calc3M( std::string );
    bool                _calcMedian( std::string );
    bool                _calcStddev( std::string );
};
#endif
