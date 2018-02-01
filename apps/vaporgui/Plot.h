//************************************************************************
//                                                                      *
//           Copyright (C)  2016                                        *
//     University Corporation for Atmospheric Research                  *
//           All Rights Reserved                                        *
//                                                                      *
//************************************************************************/
//
//  File:       Plot.h
//
//  Author:     Samuel Li
//              National Center for Atmospheric Research
//              PO 3000, Boulder, Colorado
//
//  Date:       January 2018
//

#ifdef WIN32
    #pragma warning(disable : 4100)
#endif

#ifndef PLOT_H
    #define PLOT_H

    #include <vector>
    #include <qdialog.h>
    #include <QWidget>
    #include <QLineEdit>
    #include <QCheckBox>
    #include <Python.h>
    #include "ui_errMsg.h"
    #include "ui_plotWindow.h"
    #include "RangeController.h"
    #include <vapor/DataMgr.h>
    #include <vapor/ControlExecutive.h>
    #include "PlotParams.h"

using namespace VAPoR;

// Sam: what's this?
class pErrMsg : public QDialog, public Ui_ErrMsg {
    Q_OBJECT
public:
    pErrMsg() { setupUi(this); }
};

class Plot : public QDialog, public Ui_PlotWindow {
    Q_OBJECT

public:
    /// Constructor
    Plot(QWidget *parent);
    /// Destructor
    ~Plot();

    /// Pass in the ControlExec pointer.
    ///   Note: this is the only reference for a Plot class to retrieve other
    ///         VAPoR information, e.g., Params, DataMgr, etc.

    /* put it in constructor */
    int initControlExec(VAPoR::ControlExec *ce);

    /// Something QT requires? I'm not sure...
    void showMe();

    /// This is called whenever there's a change to the parameters.
    void Update();

protected:
    /// Connect signals from widgets with slots
    void Connect();

private slots:
    /// Update list of enabled variables upon add/remove events
    void _newVarChanged(int);
    void _removeVarChanged(int);

    /// Clean up everything when data source is changed
    void _dataSourceChanged(int);

    /// Clean up data points for plotting when the following events happen
    void _spaceTimeModeChanged(bool);
    void _spaceModeP1P2Changed();
    void _spaceModeTimeChanged();
    void _timeModePointChanged();
    void _timeModeT1T2Changed();
    void _fidelityChanged();

    /// Plot when the plot button is clicked
    void _plotClicked();

private:
    VAPoR::ControlExec *_controlExec;
};

#endif    // PLOT_H
