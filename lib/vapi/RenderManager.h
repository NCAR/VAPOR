#pragma once

#include <vapor/VPCommon.h>

//! \class RenderManager
//! \ingroup VAPI
//! \brief Manages rendering visualizers

class RenderManager {
    ControlExec *_controlExec;
    GLManager *  _glManager = nullptr;

public:
    RenderManager(ControlExec *ce);
    ~RenderManager();
    int Render(String imagePath);
    void SetResolution(int width, int height);
    vector<int> GetResolution() const;
    String GetWinName() const;

private:
    void             getNearFarDist(const double posVec[3], const double dirVec[3], double &boxNear, double &boxFar);
    void             setUpProjMatrix();
    void             setUpModelViewMatrix();
    ViewpointParams *getViewpointParams() const;
};
