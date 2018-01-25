//************************************************************************
//                                                                      *
//           Copyright (C)  2016                                        *
//     University Corporation for Atmospheric Research                  *
//           All Rights Reserved                                        *
//                                                                      *
//************************************************************************/
//
//  File:       plot.h
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
    Plot(QWidget *parent);
    ~Plot();

    int initControlExec(VAPoR::ControlExec *ce);
    void showMe();
    void Update();

  protected:
    bool Connect();

  private slots:

  private:
    VAPoR::ControlExec *_controlExec;
};

#endif // PLOT_H
