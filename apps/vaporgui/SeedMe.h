//************************************************************************
//
//                   Copyright (C)  2016
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//************************************************************************/
//
//      File:           seedMe.h
//
//                      Scott Pearse
//                      National Center for Atmospheric Research
//                      PO 3000, Boulder, Colorado
//
//
//      Date:           July 2016

#ifndef SEEDME_H
#define SEEDME_H

#include <QDialog>
#include <QtCore>
#include <QtGui>
//#include <Python.h>
#include "ui_errMsg.h"
#include "ui_SeedMeGUI.h"
#include "ui_firstTimeUser.h"

namespace VAPoR {

class FirstTimeDialog : public QDialog, public Ui_FirstTimeDialog {
    Q_OBJECT
  public:
    FirstTimeDialog(QWidget *parent);
    ~FirstTimeDialog();
    QString getAPIKey() { return _apiKey; }
  public slots:
    //apiKeySet
    void letsGo();
    void showWarning() { warning->show(); }

  private:
    QString _apiKey;
    QDialog *warning;
    QVBoxLayout *layout;
    QLabel *warnLabel;
};

class smErrMsg : public QDialog, public Ui_ErrMsg {
    Q_OBJECT
  public:
    smErrMsg() { setupUi(this); }
};

class SeedMe : public QWidget, public Ui_SeedMeGUI {
    Q_OBJECT

  public:
    SeedMe();
    void Initialize() {
        cout << "Implement SeedMe Initialize() method!" << endl;
    }
    ~SeedMe();
    void CheckAPIKey();

  private:
    void connectWidgets();
    std::string buildImagesString();
    int affirmTitles();

    std::string _script;
    std::string _shareWith;
    std::string _permissions;
    std::string _videoTitle;
    std::string _link;
    std::string _outputFile;
    std::string _frameRate;
    std::string _quality;
    std::string _apiKey;
    std::string _seedmeUploadScript;
    std::vector<std::string> _images;
    FirstTimeDialog *_firstTime;
    QImage *_image;
    smErrMsg *_errMsg;
    //		PyObject *_pFunc;
    bool _isInitialized;
    bool _titleWasBlank;

    static bool _isInitializedPython; // static one-time python initialization

    int init();
    int initPython();
    void errReport(std::string msg) const;
    std::string pyErr() const;
    int getUploadScript();

  private slots:
    void showFirstTimeDialog();
    void shareEditChanged();
    void publicPressed();
    void groupPressed();
    void privatePressed();
    void titleChanged();
    void updateAPIKey();
    void updateVideoLink(std::string);
    void cancel();
    void go();
    void qualityComboChanged(int);
    void framerateComboChanged(int);
    void outputFilenameChanged();
    void selectImages();
    void selectOutFile();
    void removeImages();
};

}; // namespace VAPoR
#endif
