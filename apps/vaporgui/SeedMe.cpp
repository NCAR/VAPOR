//************************************************************************
//
//                   Copyright (C)  2016
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//************************************************************************/
//
//      File:           seedMe.cpp
//
//                      Scott Pearse
//                      National Center for Atmospheric Research
//                      PO 3000, Boulder, Colorado
//
//      Date:           July 2016
//
//

#include <Python.h>
#include <QDebug>
#include <QFileDialog>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include <vapor/GetAppPath.h>
#include <vapor/CFuncs.h>
//#include <vapor/MyPython.h>
#include "SeedMe.h"
#include "ui_SeedMeGUI.h"

using namespace std;
// using namespace VetsUtil;
using namespace VAPoR;

namespace {

template<typename T> string ToString(T val)
{
    stringstream stream;
    stream << val;
    return stream.str();
}

// Make a python list of strings from a string vector
//
PyObject *makelist(const vector<string> &v)
{
    PyObject *l = PyList_New(v.size());
    if (!l) return (NULL);
    for (size_t i = 0; i != v.size(); ++i) { PyList_SET_ITEM(l, i, PyString_FromString(v[i].c_str())); }
    return l;
}

bool fexists(string filename)
{
    ifstream ifs(filename.c_str());
    return ifs.good();
}

bool apiFileExists()
{
    QString qhome = QDir::homePath();
    assert(!qhome.isEmpty());    // Qt API guarantees an absolute home path

    qhome = QDir::toNativeSeparators(qhome);

    // Check for ~/.seedme
    //
    QString qpath = qhome;
    qpath.append("/.seedme");

    string path = qpath.toStdString();    // Convert to native OS path
    if (fexists(path)) return (true);

    // Now check for ~/seedme.txt
    //
    qpath = qhome;
    qpath.append("/seedme.txt");

    path = qpath.toStdString();    // Convert to native OS path
    if (fexists(path)) return (true);

    return (false);
}
};    // namespace

// Static member variable used to ensure Python only gets initialized
// once. Python doesn't seem to be capable of proper cleanup :-(
//
bool SeedMe::_isInitializedPython = false;

// Fetch SeedMe Python upload script into _seedmeUploadScript
//
int SeedMe::getUploadScript()
{
    _seedmeUploadScript.clear();

    // Get path to python scripts
    //
    vector<string> paths;
    paths.push_back("python");
    paths.push_back("seedmeUploadFiles.py");
    string filename = Wasp::GetAppPath("VAPOR", "share", paths);

    //
    // Read the file
    //
    ifstream in;
    in.open(filename.c_str());
    if (!in) {
        errReport("Failed to open file " + filename);
        return (-1);
    }

    std::ostringstream contents;
    contents << in.rdbuf();
    if (!in) {
        errReport("Failed to open file " + filename);
        return (-1);
    }

    in.close();

    _seedmeUploadScript = contents.str();

    return (0);
}

// Constructor
//
SeedMe::SeedMe()
{
    _firstTime = NULL;
    _errMsg = NULL;
    //	_pFunc = NULL;
    _image = NULL;
    _isInitialized = false;
    _titleWasBlank = true;

    setWindowTitle("SeedMe");
    setupUi(this);
    connectWidgets();
}

SeedMe::~SeedMe()
{
    if (_firstTime) delete _firstTime;
    if (_errMsg) delete _errMsg;
    //	if (_pFunc) Py_DECREF(_pFunc);
    if (_image) delete _image;
}

int SeedMe::initPython()
{
    if (_isInitializedPython) return (0);    // Static one time initialization!

    // Ugh. Have to define a python object to enable capturing of
    // stderr to a string. Python API doesn't support a version of
    // PyErr_Print() that fetches the error to a C++ string. Give me
    // a break!
    //
    std::string stdErr = "import sys\n"
                         "class CatchErr:\n"
                         "	def __init__(self):\n"
                         "		self.value = 'SeedMe: '\n"
                         "	def write(self, txt):\n"
                         "		self.value += txt\n"
                         "	def flush(self):\n"
                         "		pass\n"
                         "catchErr = CatchErr()\n"
                         "sys.stderr = catchErr\n"
                         "sys.stdout = catchErr\n";

    // Use MyPython singleton class to initialize Python interpeter to
    // ensure it only gets initialized once.
    //
    //	MyPython::Instance()->Initialize();

    // Catch stderr from Python to a string
    //
    int rc = PyRun_SimpleString(stdErr.c_str());
    if (rc < 0) {
        errReport(pyErr());
        return (-1);
    }

    PyObject *pMain = PyImport_AddModule("__main__");
    if (!pMain) {
        errReport(pyErr());
        return (-1);
    }

    // Create a new module object
    PyObject *pModule = PyModule_New("seedme_upload");
    if (!pModule) {
        errReport(pyErr());
        return (-1);
    }

    rc = PyModule_AddStringConstant(pModule, "__file__", "");
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pModule);
        return (-1);
    }

    //	_pFunc = MyPython::CreatePyFunc(
    //		"seedme_upload", "uploadFiles", _seedmeUploadScript.c_str()
    //	);
    //	if (! _pFunc) return(-1);

    _isInitializedPython = true;

    Py_DECREF(pModule);

    return (0);
}

int SeedMe::init()
{
    if (_isInitialized) return (0);

    _errMsg = new smErrMsg;

    // Get upload script
    //
    int rc = getUploadScript();
    if (rc < 0) return (-1);

    // One-time python initialization
    //
    rc = initPython();
    if (rc < 0) return (-1);

    _isInitialized = true;
    return (0);
}

void SeedMe::CheckAPIKey()
{
    // Hide API Key entry tools as decided on 10/17/2016
    //
    APIKeyButton->hide();
    APIKeyEdit->hide();

    int rc = init();
    if (rc < 0) return;

    vector<string> paths;
    paths.push_back("images");
    string appPath = Wasp::GetAppPath("VAPOR", "share", paths);
    appPath += "/SeedMeImage.png";

    // Seedme icon image. What happens if this fails?
    //
    _image = new QImage(QString::fromStdString(appPath));

    if (!apiFileExists()) {
        showFirstTimeDialog();

        // If still doesn't exist bail out
        //
        if (!apiFileExists()) {
            _firstTime->showWarning();
            return;
        }
    }

    SeedMeLabel->setPixmap(QPixmap::fromImage(*_image));

    show();
    raise();
    activateWindow();

    SeedMeTabWidget->setCurrentIndex(0);
}

FirstTimeDialog::FirstTimeDialog(QWidget *parent) : QDialog(parent), Ui_FirstTimeDialog()
{
    setupUi(this);

    warning = new QDialog(this);
    layout = new QVBoxLayout;
    warnLabel = new QLabel(this);
    warnLabel->setText("You must provide\n an API Key first.");
    layout->addWidget(warnLabel);
    warning->setLayout(layout);

    // connect(browseAPIKeyButton, SIGNAL(pressed()), this, SLOT(browseAPIKey()));
    connect(letsGoButton, SIGNAL(clicked()), this, SLOT(letsGo()));
}

void FirstTimeDialog::letsGo()
{
    // if (_apiKey=="") warning->show();
    // else QDialog::accept();
    QDialog::accept();
}

FirstTimeDialog::~FirstTimeDialog()
{
    if (warning) delete warning;
    // if (layout) delete layout;
    // if (warnLabel) delete warnLabel;
}

void SeedMe::showFirstTimeDialog()
{
    if (!_firstTime) _firstTime = new FirstTimeDialog(this);

    // hide api key entry widgets after decision to remove them on 10/17/2016
    //
    _firstTime->browseAPIKeyButton->hide();
    _firstTime->label_5->hide();
    _firstTime->firstTimeAPIKey->hide();
    _firstTime->label->setPixmap(QPixmap::fromImage(*_image));
    if (!_firstTime->exec()) close();
    _firstTime->raise();
    _firstTime->activateWindow();
}

void SeedMe::connectWidgets()
{
    // Advanced tab connections
    connect(PublicRadioButton, SIGNAL(pressed()), this, SLOT(publicPressed()));
    connect(GroupRadioButton, SIGNAL(pressed()), this, SLOT(groupPressed()));
    connect(PrivateRadioButton, SIGNAL(pressed()), this, SLOT(privatePressed()));
    connect(TitleEdit, SIGNAL(textChanged()), this, SLOT(titleChanged()));
    connect(APIKeyButton, SIGNAL(pressed()), this, SLOT(updateAPIKey()));

    // Basic tab connections
    connect(ShareEdit, SIGNAL(textChanged()), this, SLOT(shareEditChanged()));
    connect(GoButton, SIGNAL(clicked()), this, SLOT(go()));
    connect(FramerateCombobox, SIGNAL(valueChanged(int)), this, SLOT(framerateComboChanged(int)));
    connect(OutputFilenameTextEdit, SIGNAL(textChanged()), this, SLOT(outputFilenameChanged()));
    connect(SelectImagesButton, SIGNAL(clicked()), this, SLOT(selectImages()));
    connect(RemoveImagesButton, SIGNAL(clicked()), this, SLOT(removeImages()));
    connect(OutputFileButton, SIGNAL(clicked()), this, SLOT(selectOutFile()));
    connect(QualityComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(qualityComboChanged(int)));
    connect(HelpButton, SIGNAL(clicked()), this, SLOT(showFirstTimeDialog()));
}

void SeedMe::selectOutFile()
{
    QFileDialog *fd = new QFileDialog;
    fd->setDefaultSuffix(".mp4");
    QString defaultOpenLocation = QDir::homePath();
    if (!_outputFile.empty()) defaultOpenLocation = QString::fromStdString(_outputFile);

    QString file = fd->getSaveFileName(this, "Select Output File Name", defaultOpenLocation, tr("(*.mp4)"));
    delete fd;

    if (file.isEmpty()) return;

    OutputFilenameTextEdit->setText(file);
    _outputFile = file.toStdString();
}

void SeedMe::cancel()
{
    // Send interrupt to SeedMe server here
    //
}

int SeedMe::affirmTitles()
{
    if (_images.empty()) {
        errReport("No input image file names specified");
        return (-1);
    }

    if (_outputFile == "") {
        errReport("No output .mp4 file name specified");
        return (-1);
    }

    // SeedMe API automatically appends .mp4 suffix
    //
    string                 ext = ".mp4";
    std::string::size_type n = _outputFile.rfind(".mp4");
    if (n != std::string::npos) { _outputFile.replace(n, ext.size(), ""); }

    // Display true output path with extension
    //
    OutputFilenameTextEdit->setText(QString::fromStdString(_outputFile + ext));

    // Verify if we're overwriting existing video files
    //
    if (std::ifstream(_outputFile.c_str())) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Are you sure?");
        msgBox.setText("Target output file already exists. Are you sure to overwrite?");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) { return (-1); }
    }

    if (_videoTitle == "") {
        _titleWasBlank = true;
        string volume, dir, file;
        //		VetsUtil::Splitpath(_outputFile, volume, dir, file, false);

        if (file.rfind(".mp4") != std::string::npos) { file.erase(file.rfind(".mp4")); }

        _videoTitle = file;

        TitleEdit->blockSignals(true);
        TitleEdit->setText(QString::fromStdString(file));
        TitleEdit->blockSignals(false);
    }

    if (_frameRate.empty()) { _frameRate = "10"; }

    if (_permissions.empty()) { _permissions = "private"; }

    if (_quality.empty()) { _quality = "high"; }
    return (0);
}

void SeedMe::go()
{
    // Make sure we have valid parameters for upload script
    //
    int rc = affirmTitles();
    if (rc < 0) return;

    // Construct arguments for SeedMe Python upload script
    //
    PyObject *pArgs = PyTuple_New(8);
    if (!pArgs) {
        errReport(pyErr());
        return;
    }

    LinkLabel->setStyleSheet("QLabel {font-weight: bold; color : red; }");
    LinkLabel->setText("Processing...");
    LinkLabel->repaint();
    qApp->processEvents();

    rc = PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(_videoTitle.c_str()));
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    rc = PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(_frameRate.c_str()));
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    rc = PyTuple_SetItem(pArgs, 2, PyUnicode_FromString(_permissions.c_str()));
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    rc = PyTuple_SetItem(pArgs, 3, PyUnicode_FromString(_shareWith.c_str()));
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    rc = PyTuple_SetItem(pArgs, 4, (!_apiKey.empty()) ? PyUnicode_FromString(_apiKey.c_str()) : Py_None);
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    rc = PyTuple_SetItem(pArgs, 5, PyUnicode_FromString(_outputFile.c_str()));
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    rc = PyTuple_SetItem(pArgs, 6, PyUnicode_FromString(_quality.c_str()));
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    // All this code necessary to pass a list of strings to Python. Yuk!
    //
    PyObject *mylist = makelist(_images);
    if (!mylist) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    PyObject *arglist = Py_BuildValue("O", mylist);
    if (!arglist) {
        Py_DECREF(mylist);
        Py_DECREF(pArgs);
        errReport(pyErr());
        return;
    }

    rc = PyTuple_SetItem(pArgs, 7, arglist);
    Py_DECREF(arglist);
    if (rc < 0) {
        errReport(pyErr());
        Py_DECREF(pArgs);
        return;
    }

    //	PyObject *pValue = PyObject_CallObject(_pFunc, pArgs);
    PyObject *pValue;
    Py_DECREF(pArgs);
    if (!pValue) {
        errReport(pyErr());

        return;
    }

    // Can't decrement this until now. Who knows why?
    //
    Py_DECREF(mylist);

    // Parse return values from script (a list of two strings);
    //
    char *s1 = NULL, *s2 = NULL;
    PyArg_ParseTuple(pValue, "ss", &s1, &s2);
    Py_DECREF(pValue);

    string url = s1;
    string msg = s2;

    // on failure upload script returns a non-empty msg string
    ///
    if (!msg.empty()) {
        errReport(msg);
    } else {
        updateVideoLink(url);
    }
}

void SeedMe::publicPressed()
{
    ShareEdit->setEnabled(true);
    _permissions = "public";
}

void SeedMe::groupPressed()
{
    ShareEdit->setEnabled(true);
    _permissions = "group";
}

void SeedMe::privatePressed()
{
    ShareEdit->setEnabled(false);
    _permissions = "private";
}

void SeedMe::shareEditChanged() { _shareWith = ShareEdit->toPlainText().toStdString(); }

void SeedMe::titleChanged()
{
    _videoTitle = TitleEdit->toPlainText().toStdString();

    _titleWasBlank = _videoTitle.empty() ? true : false;
}

void SeedMe::updateAPIKey() {}

void SeedMe::updateVideoLink(string s)
{
    LinkLabel->setStyleSheet("QLabel {font-weight: bold; color : green; }");
    LinkLabel->setText("Done!  Video link:");
    LinkBrowser->setText(QString::fromStdString(s));
}

void SeedMe::qualityComboChanged(int val)
{
    _quality = QualityComboBox->itemText(val).toStdString();    // val.toStdString();
}

void SeedMe::framerateComboChanged(int val) { _frameRate = ToString(val); }

void SeedMe::outputFilenameChanged()
{
    _outputFile = OutputFilenameTextEdit->toPlainText().toStdString();

    // If user selected a title we're done. Otherwise generate a title
    //
    if (!_titleWasBlank) return;

    string volume, dir, file;
    //	VetsUtil::Splitpath(_outputFile, volume, dir, file, false);

    if (file.rfind(".mp4") != std::string::npos) { file.erase(file.rfind(".mp4")); }

    _videoTitle = file;

    TitleEdit->blockSignals(true);
    TitleEdit->setText(QString::fromStdString(_videoTitle));
    TitleEdit->blockSignals(false);
}

void SeedMe::selectImages()
{
    QString defaultOpenLocation = QDir::homePath();
    if (!_images.empty()) {
        QString img = QString::fromStdString(_images[0]);
        defaultOpenLocation = img.section('/', 0, -2);
    }
    QStringList files = QFileDialog::getOpenFileNames(this, "Select images", defaultOpenLocation, tr("Image Files (*.png *.jpg *.tif *.tiff)"));

    int size = files.size();
    if (size > 0) {
        _images.clear();
        ImagesListWidget->clear();
        for (int i = 0; i < size; i++) {
            _images.push_back(files[i].toStdString());
            ImagesListWidget->addItem(files[i]);
        }
    }
}

void SeedMe::removeImages()
{
    QList<QListWidgetItem *> selection;
    selection = ImagesListWidget->selectedItems();
    int size = selection.size();
    for (int i = 0; i < size; i++) { _images.erase(std::remove(_images.begin(), _images.end(), selection[i]->text().toStdString()), _images.end()); }
    qDeleteAll(ImagesListWidget->selectedItems());
}

// Fetch an error message genereated by Python API.
//
string SeedMe::pyErr() const
{
    PyObject *pMain = PyImport_AddModule("__main__");

    PyObject *catcher = NULL;
    if (pMain && PyObject_HasAttrString(pMain, "catchErr")) { catcher = PyObject_GetAttrString(pMain, "catchErr"); }

    // If catcher is NULL the Python message will be written
    // to stderr. Otherwise it is writter to the catchErr object.
    //
    PyErr_Print();

    if (!catcher) {
        cerr << "CATCHER NULL" << endl;
        return ("No Python error catcher");
    }

    PyObject *output = PyObject_GetAttrString(catcher, "value");
    return (PyString_AsString(output));
}

void SeedMe::errReport(string msg) const
{
    // cerr << "SeedMe Error : " << msg << endl;

    _errMsg->errorList->setText(QString::fromStdString(msg));
    _errMsg->show();
    _errMsg->raise();
    _errMsg->activateWindow();
}
