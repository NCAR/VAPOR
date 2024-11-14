#include "QtVizWinGLContextManager.h"
#include "VizWinMgr.h"
#include "VizWin.h"

void QtVizWinGLContextManager::Activate(const string &visualizerName)
{
    _vizWinMgr->Get(visualizerName)->makeCurrent();
}