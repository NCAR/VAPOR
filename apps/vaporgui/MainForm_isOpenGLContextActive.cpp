#include "MainForm.h"
#include <QOpenGLContext>

// This is to address a warning caused by QOpenGLContext and vapor/glutil.h
// being imported in the same file.

bool MainForm::isOpenGLContextActive() const { return (bool)QOpenGLContext::currentContext(); }
