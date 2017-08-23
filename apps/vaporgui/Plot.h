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
//  Author:     Scott Pearse
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:       September 2016
//
//  Description:    Implements the matPlotLib Plot class.
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
    #include <errMsg.h>
    #include <plotWindow.h>
    #include <vapor/DataMgr.h>
    #include <vapor/ControlExecutive.h>
    #include "RangeController.h"
    #include "PlotParams.h"

using namespace std;

//
//! \class Plot
//! \brief Plot class for implementing matplotlib graphs
//!
//! \author Scott Pearse
//! \version $revision
//! \date $Date$
//!
//!

class VizWinMgr;

// Override QLineEdit's focusOutEvent to add min/max extent
// validation when the user clicks away from the text box
class NewLineEdit : public QLineEdit {
    Q_OBJECT

    // using QLineEdit::QLineEdit;

public:
    NewLineEdit(QWidget *parent, bool minOrMax) : QLineEdit(parent), _minOrMax(minOrMax) { (void)_minOrMax; }

private:
    bool _minOrMax;
    void focusOutEvent(QFocusEvent *e) { QLineEdit::focusOutEvent(e); }
};

class pErrMsg : public QDialog, public Ui_ErrMsg {
    Q_OBJECT
public:
    pErrMsg() { setupUi(this); }
};

class absSpinBox : public QAbstractSpinBox {
public:
    void setValue(int i) {}
    void setValue(double d) {}
    void setText(QString s) {}
};

class Plot : public QDialog, public Ui_PlotWindow {
    Q_OBJECT

public:
    Plot(QWidget *parent);
    ~Plot();

    void Initialize(VAPoR::ControlExec *ce, VizWinMgr *vwm);

private:
    bool           init();
    void           reject();
    void           print(bool doSpace) const;
    void           showMe();
    void           initTables();
    void           initTimes();
    void           initExtents(int ts);
    void           initPlotDlg();
    void           initVariables();
    void           initConstCheckboxes();
    void           initSSCs();
    void           initCRatios();
    void           initRefinement();
    void           enableZControllers(bool s);
    void           destroyControllers();
    vector<string> getEnabledVars() const;

    PyObject *createPyFunc(string moduleName, string funcName, string script) const;

    int  initPython();
    void finalizePython();
    void updateValues(QSlider *slider, QAbstractSpinBox *spinner, QTableWidgetItem *twi);

    int findNyquist(const size_t minv[3], const size_t maxv[3], const double minu[3], const double maxu[3], double &dX, double &dY, double &dZ) const;

    void fudgeVoxBounds(size_t minv[3], size_t maxv[3]) const;

    void getSpatialExtents(double minu[3], double maxu[3], size_t &ts) const;

    int getSpatialVectors(const vector<string> vars, map<string, vector<float>> &data, map<string, vector<float>> &iData) const;

    void getTemporalExtents(double xyz[3], size_t &ts0, size_t &ts1) const;

    int getTemporalVectors(const vector<string> vars, map<string, vector<float>> &data, map<string, vector<float>> &iData) const;

    string readPlotScript() const;

    PyObject *buildNumpyArray(const vector<float> &vec) const;

    PyObject *buildPyDict(const map<string, vector<float>> &data);

    // void getSliders(
    //	QObject*& sender, QComboBox*& qcb, SpaceSSC*& x, SpaceSSC*& y,
    //	SpaceSSC*& z
    //);
    void getRanges(QObject *&sender, QComboBox *&qcb, Range *&x, Range *&y, Range *&z);

    string pyErr() const;
    void   errReport(string msg) const;

    static bool _isInitializedPython;    // static!!!!

    VAPoR::ControlExec *_controlExec;
    VAPoR::DataMgr *    _dm;
    VAPoR::PlotParams * _params;
    pErrMsg *           _errMsg;
    VizWinMgr *         _vwm;
    QDialog *           _plotDialog;
    QLabel *            _plotLabel;
    QVBoxLayout *       _plotLayout;
    QPushButton *       _plotButton;
    QImage *            _plotImage;
    vector<QCheckBox *> _spaceCheckBoxes;
    vector<QCheckBox *> _timeCheckBoxes;
    vector<string>      _vars;
    vector<string>      _vars3d;
    vector<string>      _vars2d;
    vector<string>      _uVars;
    vector<double>      _extents;
    string              _defaultImageLocation;

    vector<size_t> _cRatios;
    int            _cRatio;
    int            _refLevel;
    int            _refLevels;

    // All of the following vectors are used to store
    // spatial or temporal coordinates and extents:
    // min, max, p1, p2(if applicapble)
    vector<int> _timeExtents;

    Range *              _spaceTimeRange;
    SinglePointLineEdit *_spaceTimeLineEdit;
    TimeSlider *         _spaceTimeSlider;
    MinMaxTableCell *    _spaceTimeCell;
    MinMaxLabel *        _spaceTimeMinLabel;
    MinMaxLabel *        _spaceTimeMaxLabel;

    Range *          _spaceXRange;
    MinMaxSlider *   _spaceP1XSlider;
    MinMaxSlider *   _spaceP2XSlider;
    MinMaxLineEdit * _spaceP1XLineEdit;
    MinMaxLineEdit * _spaceP2XLineEdit;
    MinMaxTableCell *_spaceP1XCell;
    MinMaxTableCell *_spaceP2XCell;
    MinMaxLabel *    _spaceP1XMinLabel;
    MinMaxLabel *    _spaceP1XMaxLabel;
    MinMaxLabel *    _spaceP2XMinLabel;
    MinMaxLabel *    _spaceP2XMaxLabel;

    Range *          _spaceYRange;
    MinMaxSlider *   _spaceP2YSlider;
    MinMaxSlider *   _spaceP1YSlider;
    MinMaxLineEdit * _spaceP1YLineEdit;
    MinMaxLineEdit * _spaceP2YLineEdit;
    MinMaxTableCell *_spaceP1YCell;
    MinMaxTableCell *_spaceP2YCell;
    MinMaxLabel *    _spaceP1YMinLabel;
    MinMaxLabel *    _spaceP1YMaxLabel;
    MinMaxLabel *    _spaceP2YMinLabel;
    MinMaxLabel *    _spaceP2YMaxLabel;

    Range *          _spaceZRange;
    MinMaxSlider *   _spaceP1ZSlider;
    MinMaxSlider *   _spaceP2ZSlider;
    MinMaxLineEdit * _spaceP1ZLineEdit;
    MinMaxLineEdit * _spaceP2ZLineEdit;
    MinMaxTableCell *_spaceP1ZCell;
    MinMaxTableCell *_spaceP2ZCell;
    MinMaxLabel *    _spaceP1ZMinLabel;
    MinMaxLabel *    _spaceP1ZMaxLabel;
    MinMaxLabel *    _spaceP2ZMinLabel;
    MinMaxLabel *    _spaceP2ZMaxLabel;

    Range *          _timeTimeRange;
    TimeSlider *     _timeTimeMinSlider;
    TimeSlider *     _timeTimeMaxSlider;
    MinMaxLineEdit * _timeTimeMinLineEdit;
    MinMaxLineEdit * _timeTimeMaxLineEdit;
    MinMaxTableCell *_timeTimeMinCell;
    MinMaxTableCell *_timeTimeMaxCell;
    MinMaxLabel *    _timeTimeMinLabel;
    MinMaxLabel *    _timeTimeMaxLabel;

    Range *              _timeXRange;
    MinMaxSlider *       _timeXSlider;
    MinMaxLabel *        _timeXMinLabel;
    MinMaxLabel *        _timeXMaxLabel;
    SinglePointLineEdit *_timeXLineEdit;
    MinMaxTableCell *    _timeXCell;

    Range *              _timeYRange;
    MinMaxSlider *       _timeYSlider;
    MinMaxLabel *        _timeYMinLabel;
    MinMaxLabel *        _timeYMaxLabel;
    SinglePointLineEdit *_timeYLineEdit;
    MinMaxTableCell *    _timeYCell;

    Range *              _timeZRange;
    MinMaxSlider *       _timeZSlider;
    MinMaxLabel *        _timeZMinLabel;
    MinMaxLabel *        _timeZMaxLabel;
    SinglePointLineEdit *_timeZLineEdit;
    MinMaxTableCell *    _timeZCell;

    bool _triggeredByFriend;
    bool _isInitialized;

public slots:
    void go();
    //	void getPointFromRenderer();
    void newVarAdded(int index);
    void removeVar(int);
    void savePlotToFile();
    void refinementChanged(int i) { _refLevel = i; }
    void cRatioChanged(int i) { _cRatio = i; }
    void constCheckboxChanged(int state);
};

#endif    // PLOT_H
