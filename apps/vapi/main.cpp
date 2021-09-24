//#include <vapor/glutil.h>

#include <stdio.h>
#include "GLContextProvider.h"
#include "GLInclude.h"
#include "Log.h"
#include "Session.h"
#include <vapor/VAssert.h>
#include <vapor/MyBase.h>

using namespace VAPoR;
Session *_session;

int main(int argc, char **argv)
{
    Wasp::MyBase::SetErrMsgFilePtr(stderr);

    auto ctx = GLContextProvider::CreateContext();
    VAssert(ctx);
    ctx->MakeCurrent();
    LogMessage("Context: %s", glGetString(GL_VERSION));

    _session = new Session;

    _session->Load("session.vs3");
    _session->Render("out.png");

    return 0;
}
