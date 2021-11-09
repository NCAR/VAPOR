#include "GLContextProviderEGL.h"

#if Linux

    #define EGL_EGLEXT_PROTOTYPES
    #include <EGL/egl.h>
    #include <EGL/eglext.h>
    #include <GL/gl.h>

GLContextProviderEGL::GLContextEGL::GLContextEGL(void *display, void *surface, void *context) : _display(display), _surface(surface), _context(context) {}

GLContextProviderEGL::GLContextEGL::~GLContextEGL() { eglTerminate(_display); }

void GLContextProviderEGL::GLContextEGL::MakeCurrent() { eglMakeCurrent(_display, _surface, _surface, _context); }

GLContext *GLContextProviderEGL::CreateContext()
{
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint     err = eglGetError();
    if (display == nullptr || err != EGL_SUCCESS) {
        LogWarning("Failed to get a display");
        LogWarning("EGL Error: %s", stringifyEGLError(err));
        return nullptr;
    }

    return createContextForDisplay(display);
}

GLContext *GLContextProviderEGL::createContextForDisplay(void *display)
{
    EGLint     major, minor, err;
    EGLBoolean ret = eglInitialize(display, &major, &minor);
    err = eglGetError();
    if (ret != EGL_TRUE || err != EGL_SUCCESS) {
        LogWarning("Failed to initialize EGL");
        LogWarning("EGL Error: %s", stringifyEGLError(err));
        return nullptr;
    }
    LogInfo("EGL Version %i.%i", major, minor);

    EGLint       numConfigs;
    EGLConfig    config;
    const EGLint configAttribs[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT, EGL_NONE};

    eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
    err = eglGetError();
    if (err != EGL_SUCCESS) {
        LogWarning("Failed to get EGL config");
        LogWarning("EGL Error: %s", stringifyEGLError(err));
        return nullptr;
    }

    const EGLint pbufferAttribs[] = {
        EGL_WIDTH, 100, EGL_HEIGHT, 100, EGL_NONE,
    };

    EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
    err = eglGetError();
    if (surface == nullptr || err != EGL_SUCCESS) {
        LogWarning("Failed to create surface");
        LogWarning("EGL Error: %s", stringifyEGLError(err));
        return nullptr;
    }

    eglBindAPI(EGL_OPENGL_API);
    err = eglGetError();
    if (err != EGL_SUCCESS) {
        LogWarning("Failed to bind OpenGL API");
        LogWarning("EGL Error: %s", stringifyEGLError(err));
        return nullptr;
    }

    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
    err = eglGetError();
    if (context == nullptr || err != EGL_SUCCESS) {
        LogWarning("Failed to create context");
        LogWarning("EGL Error: %s", stringifyEGLError(err));
        return nullptr;
    }

    GLContextEGL *glContext = new GLContextEGL(display, surface, context);

    LogInfo("Created context: %s", glContext->GetVersion().c_str());

    return glContext;
}

const char *GLContextProviderEGL::stringifyEGLError(int e)
{
    switch (e) {
    case EGL_SUCCESS: return "EGL_SUCCESS";
    case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
    case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
    case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
    case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
    case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
    case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
    case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
    case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
    default: return "EGL_UNKNOWN_ERROR";
    }
}

#endif
