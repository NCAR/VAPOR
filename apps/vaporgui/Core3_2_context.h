#pragma once

#include <QGLContext>

#if defined(Q_OS_MAC)
    #if QT_VERSION < 0x050000 && QT_VERSION >= 0x040800    // if less than 5.0.0
void *select_3_2_mac_visual(GDHandle handle, int depthBufferSize);
    #endif
#endif

struct Core3_2_context : public QGLContext {
    Core3_2_context(const QGLFormat &format, QPaintDevice *device) : QGLContext(format, device) {}
    Core3_2_context(const QGLFormat &format) : QGLContext(format) {}

#if defined(Q_OS_MAC)
    #if QT_VERSION < 0x050000 && QT_VERSION >= 0x040800    // if less than 5.0.0
    virtual void *chooseMacVisual(GDHandle handle) { return select_3_2_mac_visual(handle, this->format().depthBufferSize()); }
    #endif
#endif
};
