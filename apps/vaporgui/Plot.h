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
    void _spaceModeP1Changed();
    void _spaceModeP2Changed();
    void _spaceModeTimeChanged(int);
    void _timeModePointChanged();
    void _timeModeT1T2Changed();
    void _numberOfSamplesChanged();
    void _axisLocksChanged(int);

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

    /// All the python stuff happens here; no python outside this method
    void _invokePython(const QString &, const std::vector<std::string> &, const std::vector<std::vector<float>> &, const std::vector<float> &, const std::string &, const std::string &);

    // Returns a string with the proper X label if all variables share the same coordinate unit.
    //   Otherwise returns an empty string.
    //
    std::string _getXLabel();

    // Returns a string with the proper Y label if all variables share the same unit.
    //   Otherwise returns an empty string.
    //
    std::string _getYLabel();

    // Fix the min and max extents kept in the params class based on
    //   what extents the new variable has.
    //   If the new variable is an empty string, then update the extents
    //   using all the enabled variables.
    //
    void _fixActiveExtents(const std::string);
};

#endif    // PLOT_H
