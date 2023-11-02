//#include <vapor/glutil.h>

#include <stdio.h>
#include <osgl/GLContextProvider.h>
#include <osgl/GLInclude.h>
#include <osgl/Log.h>
#include <vapor/Session.h>
#include <vapor/VAssert.h>
#include <vapor/MyBase.h>

using namespace VAPoR;
Session *_session;

int main(int argc, char **argv)
{
    const char *sessionPath;
    if (argc == 2)
        sessionPath = argv[1];
    else 
        sessionPath = "/Users/stasj/Work/sessions/time.vs3";


    Wasp::MyBase::SetErrMsgFilePtr(stderr);

    auto ctx = GLContextProvider::CreateContext();
    VAssert(ctx);
    ctx->MakeCurrent();
    LogMessage("Context: %s", glGetString(GL_VERSION));

    _session = new Session;

    _session->Load(sessionPath);
    
    // _session->NewRenderer("Contour");
    
    _session->Render("out-vapi-test.png");

    return 0;
}
