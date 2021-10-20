#include "GLContextProviderMacOS.h"
#include <AppKit/NSOpenGL.h>
#include <OpenGL/CGLContext.h>
#include "VPCommon.h"

GLContextProviderMacOS::GLContextMacOS::GLContextMacOS(void *ctx)
: _ctx(ctx) {}

GLContextProviderMacOS::GLContextMacOS::~GLContextMacOS()
{
    NSOpenGLContext *ctx = (NSOpenGLContext *)_ctx;
    [ctx dealloc];
}

void GLContextProviderMacOS::GLContextMacOS::MakeCurrent()
{
    NSOpenGLContext *ctx = (NSOpenGLContext *)_ctx;
    [ctx makeCurrentContext];
}

GLContext *GLContextProviderMacOS::CreateContext()
{
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core, // Undocumented
        0
    };
    
    NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (fmt == nil) {
        LogWarning("Failed to create pixel format\n");
        return nullptr;
    }
    
    NSOpenGLContext *ctx = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
    if (ctx == nil) {
        LogWarning("Failed to create context\n");
        [fmt dealloc];
        return nullptr;
    }
    
    GLContextMacOS *glContext = new GLContextMacOS(ctx);
    LogInfo("Created context: %s", glContext->GetVersion().c_str());
    
    return glContext;
}
