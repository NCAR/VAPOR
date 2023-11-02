#include <osgl/GLContextProvider.h>
#include <osgl/GLInclude.h>

int main(int argc, char **argv)
{
    auto ctx = OSGL::GLContextProvider::CreateContext();
    assert(ctx);
    ctx->MakeCurrent();
    printf("OpenGL Context: %s\n", glGetString(GL_VERSION));
    return 0;
}
