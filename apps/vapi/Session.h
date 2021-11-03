#pragma once

#include "VPCommon.h"

class RenderManager;

//! \class Session
//! \ingroup VAPI
//! \brief Manages a Vapor Session
//! \author Stas Jaroszynski

class Session {
public:
    ControlExec *  _controlExec;
    RenderManager *_renderManager;

    Session();
    ~Session();
    void CloseDataset(String name);
    void CloseAllDatasets();
    int  OpenDataset(String name, String format, const vector<String> &files, const vector<String> &options);
    int  OpenDataset(String format, const vector<String> &files, String name = "");
    int  Load(String path);
    void Reset();

    int  Render(String imagePath);
    void SetTimestep(int ts);

private:
    void             loadAllParamsDatasets();
    void             getParamsDatasetInfo(String name, String *type, vector<String> *files);
    GUIStateParams * getGUIStateParams() const;
    AnimationParams *getAnimationParams() const;
};
