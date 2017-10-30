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
}    // namespace VAPoR

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
    int  initControlExec(VAPoR::ControlExec *ce);
    void showMe();
    int  initialize();    // connecting slots
    void Update(VAPoR::StatisticsParams *sParams);

protected:
    // Keeps the current variables shown and their statistical values.
    // Invalid values are stored as std::nan("1");
    //
    class ValidStats {
    public:
        bool addVariable(std::string);
        // bool removeVariable( std::string );

        bool add3MStats(std::string, const double *);    // Min, Max, Mean
        bool addMedian(std::string, double);
        bool addStddev(std::string, double);

        // invalid values are represented as nan.
        bool get3MStats(std::string, double *);
        bool getMedian(std::string, double *);
        bool getStddev(std::string, double *);

        bool invalidAll();

    private:
        std::vector<std::string> _variables;
        std::vector<double>      _values[5];    // 0: min
                                                // 1: max
                                                // 2: mean
                                                // 3: median
                                                // 4: stddev
        int getVarIdx(std::string);             // -1: not exist
                                                // >=0: a valid index
    };                                          // finish ValidStats

private slots:
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
    sErrMsg *_errMsg;

    VAPoR::ControlExec *_controlExec;
    VAPoR::DataStatus * _dataStatus;
    VAPoR::DataMgr *    _dmgr;
    VAPoR::Grid *       _rGrid;
};
#endif
