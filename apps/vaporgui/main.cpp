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
#include <vapor/OptionParser.h>
#include <vapor/FileUtils.h>
#include <vapor/OSPRay.h>
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

struct opt_t {
    OptionParser::Boolean_T    help;
    OptionParser::Boolean_T    render;
    OptionParser::IntRange_    renderRange;
    OptionParser::Dimension2D_ resolution;
    char *                     outputPath;
} opt;
OptionParser::OptDescRec_T set_opts[] = {{"help", 0, "", "Print this message and exit"},
                                         {"render", 0, "", "Render a given session file and exit"},
                                         {"timesteps", 1, "0:0", "Timesteps to render when using -render. Defaults to all timesteps."},
                                         {"resolution", 1, "1920x1080", "Output resolution when using -render"},
                                         {"output", 1, "vapor.tiff", "Output image file when using -render. This will be suffixed with the timestep number. Supports tiff, jpg"},
                                         {NULL}};
OptionParser::Option_T     get_options[] = {{"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {"render", Wasp::CvtToBoolean, &opt.render, sizeof(opt.render)},
                                        {"timesteps", Wasp::CvtToIntRange, &opt.renderRange, sizeof(opt.renderRange)},
                                        {"resolution", Wasp::CvtToDimension2D, &opt.resolution, sizeof(opt.resolution)},
                                        {"output", Wasp::CvtToString, &opt.outputPath, sizeof(opt.outputPath)},
                                        {NULL}};
const char *               ProgName;

QApplication *app;
int           main(int argc, char **argv)
{
    OptionParser op;
    ProgName = FileUtils::LegacyBasename(argv[0]);
    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }
    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }
    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] [session.vs3] [data files...]" << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

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

    VOSP::Initialize(&argc, argv);

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

    vector<QString> files;
    for (int i = 1; i < argc; i++) { files.push_back(argv[i]); }
    MainForm *mw = new MainForm(files, app);

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

    int estatus = 0;
    if (opt.render) { estatus = mw->RenderAndExit(opt.renderRange.min, opt.renderRange.max, opt.outputPath, opt.resolution.nx, opt.resolution.ny); }

    if (estatus == 0) { estatus = a.exec(); }

    VOSP::Shutdown();
    if (diagfp) fclose(diagfp);
    if (errfp) fclose(errfp);
    exit(estatus);
}
