#include <QGLContext>

#if QT_VERSION < 0x050000 // if less than 5.0.0
void* select_3_2_mac_visual(GDHandle handle, int depthBufferSize)
{
    static const int Max = 40;
    NSOpenGLPixelFormatAttribute attribs[Max];
    int cnt = 0;

    attribs[cnt++] = NSOpenGLPFAOpenGLProfile;
    attribs[cnt++] = NSOpenGLProfileVersion3_2Core;

    attribs[cnt++] = NSOpenGLPFADoubleBuffer;

    attribs[cnt++] = NSOpenGLPFADepthSize;
    attribs[cnt++] = (NSOpenGLPixelFormatAttribute)(depthBufferSize==-1)?32:depthBufferSize;

    attribs[cnt] = 0;
    Q_ASSERT(cnt < Max);


    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
}
#endif
