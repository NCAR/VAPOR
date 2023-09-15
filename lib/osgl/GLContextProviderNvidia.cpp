#include <vapor/GLContextProviderNvidia.h>
#include <vapor/STLUtils.h>

#if BUILD_EGL

#if !Linux
    #error EGL only supported on linux
#endif

    #include <vapor/GLAD.h>
    #include <glad/egl.h>

//    Required if using system EGL
//    #define EGL_EGLEXT_PROTOTYPES
//    #include <EGL/eglext.h>

GLContext *GLContextProviderNvidia::CreateContext()
{
    if (!gladLoaderLoadEGL(nullptr)) {
        LogWarning("Could not load EGL");
        return nullptr;
    }
    
    const int    MAX_DEVICES = 8;
    EGLDeviceEXT eglDevices[MAX_DEVICES];
    String       eglDeviceStrings[MAX_DEVICES];
    EGLint       numDevices;

//    Required if using system EGL
//    PFNEGLQUERYDEVICESEXTPROC       eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
//    PFNEGLQUERYDEVICESTRINGEXTPROC  eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)eglGetProcAddress("eglQueryDeviceStringEXT");
//    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    if (eglQueryDevicesEXT == nullptr || eglQueryDeviceStringEXT == nullptr || eglGetPlatformDisplayEXT == nullptr) {
        LogWarning("Could not load Nvidia EGL extensions");
        return nullptr;
    }

    eglQueryDevicesEXT(MAX_DEVICES, eglDevices, &numDevices);
    for (int i = 0; i < numDevices; i++) {
        const char *str = eglQueryDeviceStringEXT(eglDevices[i], EGL_EXTENSIONS);
        eglDeviceStrings[i] = String(str);
    }

    LogInfo("Found %i EGL devices:", numDevices);
    for (int i = 0; i < numDevices; i++) Log::Info("\t[%i] = %s", i, eglDeviceStrings[i].c_str());

    int nvidiaDeviceId = -1;
    for (int i = 0; i < numDevices; i++) {
        if (STLUtils::Contains(STLUtils::ToLower(eglDeviceStrings[i]), "nvidia") ||
            STLUtils::Contains(STLUtils::ToLower(eglDeviceStrings[i]), "egl_nv_") ) {
            nvidiaDeviceId = i;
            break;
        }
    }

    if (nvidiaDeviceId >= 0) {
        LogInfo("Found Nvidia GPU [%i] %s", nvidiaDeviceId, eglDeviceStrings[nvidiaDeviceId].c_str());
    } else {
        LogWarning("Nvidia GPU not found");
        return nullptr;
    }

    EGLDisplay display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevices[nvidiaDeviceId], 0);
    EGLint     err = eglGetError();
    if (display == nullptr || err != EGL_SUCCESS) {
        LogWarning("Failed to get display from Nvidia GPU");
        LogWarning("EGL Error: %s", stringifyEGLError(err));
        return nullptr;
    }

    return createContextForDisplay(display);
}

#endif
