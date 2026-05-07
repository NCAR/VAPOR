#pragma once

#include <vapor/VPCommon.h>
#include <vapor/GLContext.h>

//! \class RenderManager
//! \ingroup VAPI
//! \brief Manages rendering visualizers

class RenderManager {
    ControlExec *_controlExec;
    GLManager *  _glManager = nullptr;
    bool         _useOSGLContext = false;
    static GLContext * _glContext;

public:
    RenderManager(ControlExec *ce, bool useOSGLContext = false);
    ~RenderManager();
    int Render(String imagePath, bool fast=false);
    void SetResolution(int width, int height);
    vector<int> GetResolution() const;
    String GetWinName() const;
    static GLContext *GetOSGLContext();

private:
    void             getNearFarDist(const double posVec[3], const double dirVec[3], double &boxNear, double &boxFar);
    void             setUpProjMatrix();
    void             setUpModelViewMatrix();
    ViewpointParams *getViewpointParams() const;
};
