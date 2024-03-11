#include <osgl/GLInclude.h>
#include <osgl/GLContextProvider.h>

using namespace OSGL;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define GL_SILENCE_DEPRECATION

#define GLERRTEST if (glGetError() != GL_NO_ERROR) printf("OPENGL ERROR\n");

int main(int argc, char **argv)
{
    printf("Framebuffer test\n");

    auto ctx = GLContextProvider::CreateContext();
    assert(ctx);
    ctx->MakeCurrent();

    LogMessage("Context: %s", glGetString(GL_VERSION));

    
    GLuint _colorBuffer;
    glGenTextures(1, &_colorBuffer);
    glBindTexture(GL_TEXTURE_2D, _colorBuffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    
    printf("[%i] About to gen framebuffer\n", __LINE__);
    GLuint _id;
    glGenFramebuffers(1, &_id);
    printf("[%i] Framebuffer generated: %i\n", __LINE__, _id);

    glBindFramebuffer(GL_FRAMEBUFFER, _id);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuffer, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    
    glViewport(0, 0, 100, 100);
    
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
    
    unsigned char *pixels[100*100*3];
    memset(pixels, 1, 100*100*3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, 100, 100, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    
    stbi_write_png("out-framebuffer-red.png", 100, 100, 3, pixels, 0);
    
    GLERRTEST;

    return 0;
}
