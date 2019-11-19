//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		main.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June 2004
//
//	Description:  Main program for vapor gui

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cerrno>
#include <qapplication.h>
#include "MainForm.h"
#include <qfont.h>
#include <QMessageBox>
#include <QFontDatabase>
#include "BannerGUI.h"
#include <vapor/CMakeConfig.h>
#include <vapor/ResourcePath.h>
#ifdef WIN32
    #include "Windows.h"
#endif
using namespace std;
using namespace VAPoR;
using namespace Wasp;
void myMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg: break;
    case QtWarningMsg: break;
    case QtFatalMsg: break;
    default:    // ignore QtCriticalMsg and QtSystemMsg
        break;
    }
}

//
// Open a file named by the environment variable, path_var. Exit on failure
//
FILE *OpenLog(string path_var)
{
    FILE *      fp = NULL;
    const char *cstr = getenv(path_var.c_str());
    if (!cstr) return (NULL);
    string s = cstr;
    if (!s.empty()) {
        fp = fopen(s.c_str(), "w");
        if (!fp) {
            cerr << "Failed to open " << s << " : " << strerror(errno) << endl;
            exit(1);
        }
        MyBase::SetDiagMsgFilePtr(fp);
    }
    return (fp);
}

#ifndef WIN32
    #warning Qt4 uses deprecated OSX calls which pollute the console with warnings
#endif
#if defined(Darwin) && !defined(NDEBUG)
    //#ifdef DEBUG
    #include <unistd.h>
int  _savedSTDERR;
void HideSTDERR()
{
    _savedSTDERR = dup(STDERR_FILENO);
    freopen("/dev/null", "w", stderr);
}
void RestoreSTDERR() { dup2(_savedSTDERR, STDERR_FILENO); }
#endif

QApplication *app;
int           main(int argc, char **argv)
{
    // Install our own message handler.
    // Needed for SGI to avoid dithering:

    // FILE *diagfp = OpenLog("VAPOR_DIAG_LOG");
    // FILE *errfp = OpenLog("VAPOR_ERR_LOG");
    FILE *diagfp = NULL;
    FILE *errfp = NULL;
    if (getenv("VAPOR_DEBUG")) MyBase::SetDiagMsgFilePtr(stderr);

#ifdef Darwin
    if (!getenv("DISPLAY")) setenv("DISPLAY", ":0.0", 0);
#endif
#ifdef Q_WS_X11
    if (!getenv("DISPLAY")) {
        fprintf(stderr, "Error:  X11 DISPLAY variable is not defined. %s \n", "Vapor user interface requires X server to be available.");
        exit(-1);
    }
#endif

#ifdef IRIX
    QApplication::setColorSpec(QApplication::ManyColor);
#endif

#if defined(Darwin) && !defined(NDEBUG)
    if (!getenv("VAPOR_DEBUG")) { HideSTDERR(); }
#endif

    QApplication a(argc, argv, true);

    // All C programs are run with the locale set to "C"
    // Initalizing QApplication changes the locale to the system configuration
    // which can cause problems if it is not supported.
    // Udunits does not support it and vapor potentially does not either.
    //
    // https://www.gnu.org/software/libc/manual/html_node/Setting-the-Locale.html
    // https://doc.qt.io/qt-5/qcoreapplication.html#locale-settings
    //
    setlocale(LC_ALL, "C");

    // Set path for Qt to look for its plugins.
    //
    QString     filePath = GetResourcePath("plugins").c_str();
    QStringList filePaths(filePath);
    QCoreApplication::setLibraryPaths(filePaths);
// For Mac and Linux, set the PYTHONHOME in this app
#ifndef WIN32

    const char *s = getenv("PYTHONHOME");
    string      phome = s ? s : "";
    if (!phome.empty()) {
        string msg("The PYTHONHOME variable is already specified as: \n");
        msg += phome;
        msg += "\n";
        msg += "The VAPOR ";
        msg += "python" + PYTHON_VERSION;
        msg += " environment will operate in this path\n";
        msg += " This path must be the location of a Python ";
        msg += "python" + PYTHON_VERSION;
        msg += " installation\n";
        msg += "Unset the PYTHONHOME environment to revert to the installed ";
        msg += "VAPOR python" + PYTHON_VERSION + " environment.";
        QMessageBox::warning(0, "PYTHONHOME warning", msg.c_str());
    } else {
        phome = GetPythonDir();
        setenv("PYTHONHOME", phome.c_str(), 1);
    }
    MyBase::SetDiagMsg("PYTHONHOME = %s", phome.c_str());

#endif

    app = &a;
    a.setPalette(QPalette(QColor(233, 236, 216), QColor(233, 236, 216)));

    vector<QString> files;
    for (int i = 1; i < argc; i++) { files.push_back(argv[i]); }
    MainForm *mw = new MainForm(files, app);

#if defined(Darwin) && !defined(NDEBUG)
    RestoreSTDERR();
#endif

    // StartupParams* sParams = new StartupParams(0);

    string fontFile = GetSharePath("fonts/arimo.ttf");

    QFontDatabase fdb;
    fdb.addApplicationFont(QString::fromStdString(fontFile));
    QStringList fonts = fdb.families();
    QFont       f = fdb.font("Arimo", "normal", 12);

    const char *useFont = std::getenv("USE_SYSTEM_FONT");
    if (!useFont) { mw->setFont(f); }

    mw->setWindowTitle("VAPOR User Interface");
    mw->show();
    // Disable banner in debug build
#ifdef NDEBUG
    std::string banner_file_name = "vapor_banner.png";
    BannerGUI   banner(mw, banner_file_name, 3000);
#endif
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    int estatus = a.exec();

    if (diagfp) fclose(diagfp);
    if (errfp) fclose(errfp);
    exit(estatus);
}
