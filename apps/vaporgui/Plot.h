//************************************************************************
//                                                                      *
//           Copyright (C)  2016                                        *
//     University Corporation for Atmospheric Research                  *
//           All Rights Reserved                                        *
//                                                                      *
//************************************************************************/
//
//  Author:     Samuel Li
//              National Center for Atmospheric Research
//              PO 3000, Boulder, Colorado
//
//  Date:       January 2018
//

#ifndef PLOT_H
#define PLOT_H

#include <vector>
#include <QWidget>
#include "ui_plotWindow.h"
#include <vapor/ParamsMgr.h>
#include <vapor/DataStatus.h>
#include "PlotParams.h"

using namespace VAPoR;

class Plot : public QDialog, public Ui_PlotWindow {
    Q_OBJECT

public:
    Plot(VAPoR::DataStatus *status, VAPoR::ParamsMgr *manager, QWidget *parent = 0);
    ~Plot();

    /// This is called whenever there's a change to the parameters.
    void Update();

private slots:
    /// Update list of enabled variables upon add/remove events
    void _newVarChanged(int);
    void _removeVarChanged(int);

    /// Clean up everything when data source is changed
    void _dataSourceChanged(int);

    /// Clean up data points for plotting when the following events happen
    // void _spaceTimeModeChanged( int );
    void _spaceModeP1Changed();
    void _spaceModeP2Changed();
    void _spaceModeTimeChanged(double);
    void _timeModePointChanged();
    void _timeModeT1T2Changed();
    void _numberOfSamplesChanged();

    /// Plot when the plot button is clicked
    void _spaceTabPlotClicked();
    void _timeTabPlotClicked();

private:
    VAPoR::DataStatus *_dataStatus;
    VAPoR::ParamsMgr * _paramsMgr;
    QDialog *          _plotDialog;
    QLabel *           _plotLabel;
    QLineEdit *        _plotPathEdit;
    QLabel *           _plotImageLabel;
    QVBoxLayout *      _plotLayout;
    QIntValidator *    _validator;

    /// Access functions to other pointers
    VAPoR::PlotParams *_getCurrentPlotParams() const;
    VAPoR::DataMgr *   _getCurrentDataMgr() const;

    void _setInitialExtents();
    void _invokePython(const QString &, const std::vector<std::string> &, const std::vector<std::vector<float>> &, const std::vector<float> &);
};

#endif    // PLOT_H
