#include <vapor/glutil.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cassert>
#include <qapplication.h>
#include <QGLWidget>
#include <vapor/OptionParser.h>
#include <vapor/ControlExecutive.h>
#include <vapor/animationparams.h>
#include <vapor/ParamsMgr.h>
#include <vapor/TwoDDataRenderer.h>
#include <vapor/TwoDDataParams.h>
#include <vapor/HelloRenderer.h>
#include <vapor/HelloParams.h>
#include <vapor/arrowparams.h>
#include "test_vizwin.h"

using namespace Wasp;
using namespace VAPoR;

struct {
    int                     nts;
    int                     ts0;
    string                  vdcmaster;
    string                  sessionfile;
    string                  var;
    OptionParser::Boolean_T arrow;
    OptionParser::Boolean_T twoD;
    OptionParser::Boolean_T hello;
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T debug;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"nts", 1, "1", "Number of timesteps to play"},
                                         {"ts0", 1, "0", "First time step to play"},
                                         {"ses", 1, "", "VAPOR session file to read"},
                                         {"var", 1, "", "Primary variable name"},
                                         {"arrow", 0, "", "Create arrow renderer"},
                                         {"twoD", 0, "", "Create TwoD renderer"},
                                         {"hello", 0, "", "Create hello renderer"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {"quiet", 0, "", "Operate quietly"},
                                         {"debug", 0, "", "Debug mode"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"nts", Wasp::CvtToInt, &opt.nts, sizeof(opt.nts)},
                                        {"ts0", Wasp::CvtToInt, &opt.ts0, sizeof(opt.ts0)},
                                        {"ses", Wasp::CvtToCPPStr, &opt.sessionfile, sizeof(opt.sessionfile)},
                                        {"var", Wasp::CvtToCPPStr, &opt.var, sizeof(opt.var)},
                                        {"arrow", Wasp::CvtToBoolean, &opt.arrow, sizeof(opt.arrow)},
                                        {"twoD", Wasp::CvtToBoolean, &opt.twoD, sizeof(opt.twoD)},
                                        {"hello", Wasp::CvtToBoolean, &opt.hello, sizeof(opt.hello)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
                                        {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
                                        {NULL}};

const char *ProgName;

void ErrMsgCBHandler(const char *msg, int) { cerr << ProgName << " : " << msg << endl; }

RenderParams *CreateRender(ControlExec *CE, string winName, string className, string name)
{
    RenderParams *rp = NULL;
    ParamsMgr *   pMgr = CE->GetParamsMgr();

    int rc = CE->ActivateRender(winName, className, name, false);
    if (rc < 0) {
        cerr << "Failed to activate renderer type : " << className << endl;
        return NULL;
    }

    // Create a new barbs params to be used in rendering
    rp = pMgr->GetRenderParams(winName, className, name);

    if (!rp) {
        cerr << "Failed to get params for renderer type : " << className << endl;
        return NULL;
    }

    return (rp);
}

int CreateNew(ControlExec *CE, string winName)
{
    int rc = CE->NewVisualizer(winName);
    if (rc < 0) {
        cerr << "Failed to creat visualizer " << endl;
        return -1;
    }

    RenderParams *rParams = NULL;
    if (opt.arrow) {
        string       name = "arrow1";
        ArrowParams *myRParams = NULL;

        myRParams = (ArrowParams *)CreateRender(CE, winName, ArrowParams::m_classTag, name);

        if (!myRParams) return (-1);
        rParams = myRParams;
    }

    if (opt.twoD) {
        string          name = "twoD1";
        TwoDDataParams *myRParams = NULL;

        myRParams = (TwoDDataParams *)CreateRender(CE, winName, TwoDDataParams::m_classTag, name);

        if (!myRParams) return (-1);
        rParams = myRParams;
    }

    if (opt.hello) {
        string       name = "hello1";
        HelloParams *myRParams = NULL;

        myRParams = (HelloParams *)CreateRender(CE, winName, HelloParams::m_classTag, name);

        if (!myRParams) return (-1);

        myRParams->SetLineThickness(4.0);
        rParams = myRParams;
    }
    if (!opt.var.empty()) { rParams->SetVariableName(opt.var); }

    return (0);
}

QApplication *app;
int           main(int argc, char **argv)
{
    OptionParser op;
    double       timer = 0.0;
    string       s;

    const char *ProgName = "test_CE";
    MyBase::SetErrMsgFilePtr(stderr);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] metafiles " << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (opt.debug) { MyBase::SetDiagMsgFilePtr(stderr); }

    if (argc < 2) {
        cerr << "Usage: " << ProgName << " [options] " << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }
    // Set up Qt application
    QApplication a(argc, argv, true);
    app = &a;

    string       winName = "win1";
    ControlExec *CE = new ControlExec;
    // Create the OpenGL Window
    Test_VizWin vizwin(0, CE, winName);

    // Create and set up a visualizer (no. 0)

    bool useSession = false;
#ifdef DEAD
    CE->CreateDefaultParams(0);
    CE->SetActiveVizIndex(0);

    // Restore preferences (expect .vapor3_prefs in home directory)
    if (getenv("HOME")) {
        string prefPath = string(getenv("HOME")) + "/.vapor3_prefs";
        CE->RestorePreferences(prefPath);
    }
    // Load session if specified
    bool enabled = false;
    if (opt.sessionfile.length() > 0) {
        CE->SetToDefault();
        int rc = CE->RestoreSession(opt.sessionfile);
        if (rc == 0) useSession = true;
    }
#endif

    // Load data
    vector<string> files;
    for (int i = 1; i < argc; i++) { files.push_back(argv[i]); }

    int rc = CE->LoadData(files, 1000, !useSession);
    if (rc < 0) return (1);

        // Disable command queue (not used here)
#ifdef DEAD
    Command::blockCapture();
#endif

    rc = CreateNew(CE, winName);
    if (rc < 0) return (1);

    // Make the Window display itself:
    vizwin.show();

#ifdef DEAD
    if (useSession) {
    #ifdef DEAD
        int rc = CE->ActivateEnabledRenderers(0);
        if (rc) exit(rc);
    #endif
    } else {
        // Enable the barbs for rendering
        rParams->SetEnabled(true);
        // Activate the barbs renderer
        CE->ActivateRender(winName, HelloParams::m_classTag, "hello1", true);
    }
#endif

#ifdef DEAD
    // Set up animation and capture
    // If there is a session file, use its timestep initially, otherwise get
    // the first time step from the command argument.

    #ifdef DEAD
    AnimationParams *animParams = CE->GetParamsMgr()->GetAnimationParams(0);
    if (!useSession) animParams->setCurrentTimestep(opt.ts0);
    int frameStep = animParams->getFrameStepSize();
    // Loop through the specified number of time steps
    for (int j = 0; j < opt.nts; j++) {
        // Prepare for capture if enabled:
        if (animParams->GetCaptureEnabled()) {
            string capturePath = animParams->GetCaptureFilepath();
            if (capturePath != "") CE->EnableCapture(capturePath, 0);
        }

        // Display the scene
        printf("Rendering timestep %d of %d\n", j + 1, opt.nts);
        vizwin.updateGL();

        // Advance the frame, if more to animate
        if (j + 1 >= opt.nts) break;
        int currentTS = animParams->getCurrentTimestep();
        currentTS += frameStep;
        if (currentTS >= animParams->getMinTimestep() && currentTS <= animParams->getMaxTimestep())
            animParams->setCurrentTimestep(currentTS);
        else
            break;
    }

    #endif
    printf("Animation sequence complete\n");
#endif
    int estatus = a.exec();

    if (!opt.quiet) { fprintf(stdout, "total process time : %f\n", timer); }

    ParamsMgr *pm = CE->GetParamsMgr();
    pm->SaveToFile("file.xml");

    delete CE;

    exit(estatus);
}
