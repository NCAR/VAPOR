# OSGL

OSGL is an open source, cross-platform library designed to simplify the use of OpenGL in headless environments. It uses OS and vendor specific extensions, as well as EGL to aquire an OpenGL context without needing to go through the window manager. It will fallback to providing a software OpenGL context using OSMesa if getting a hardware context fails.

Getting an OpenGL context is now as simple as this:
```
#include <osgl/GLContextProvider.h>
#include <osgl/GLInclude.h>

int main(int argc, char **argv)
{
    auto ctx = OSGL::GLContextProvider::CreateContext();
    printf("OpenGL Context: %s\n", glGetString(GL_VERSION));
    return 0;
}
```

Since the context will not be bound to a window, you need to create and bind your own target framebuffer. An complete example can be found in `tests/test_framebuffer.cpp`.

OSGL currently supports macOS and Linux. Windows support is planned.
