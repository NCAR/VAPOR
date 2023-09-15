#include <vapor/GLContextProvider.h>
#include <vapor/GLInclude.h>

int main(int argc, char **argv)
{
    LogMessage("Version Test");
    auto ctx = GLContextProvider::CreateContext();
    assert(ctx);
    ctx->MakeCurrent();
    LogMessage("Context: %s", glGetString(GL_VERSION));
    return 0;
}
