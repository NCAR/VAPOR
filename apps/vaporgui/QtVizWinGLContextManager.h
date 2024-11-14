#pragma once
#include <vapor/VisualizerGLContextManager.h>

class VizWinMgr;
class VizWin;

class QtVizWinGLContextManager : public VAPoR::VisualizerGLContextManager {
    VizWinMgr *_vizWinMgr;
public:
    QtVizWinGLContextManager(VizWinMgr *vizWinMgr) : _vizWinMgr(vizWinMgr) {}
    void Activate(const string &visualizerName) override;
};
