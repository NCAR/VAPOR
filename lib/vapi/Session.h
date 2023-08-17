#pragma once

#include <vapor/VPCommon.h>

class RenderManager;

//! \class Session
//! \ingroup VAPI
//! \brief Manages a Vapor Session
//! \author Stas Jaroszynski

class Session {
public:
    ControlExec *  _controlExec = nullptr;
    RenderManager *_renderManager = nullptr;

    Session();
    virtual ~Session();
    void CloseDataset(String name);
    void CloseAllDatasets();
    int  OpenDataset(String name, String format, const vector<String> &files, const vector<String> &options);
    int  OpenDataset(String format, const vector<String> &files, String name);
    String OpenDataset(String format, vector<String> files);
    int  Load(String path);
    int  Save(String path);
    void Reset();
    
    vector<String> GetDatasetNames() const;
    vector<String> GetRendererNames() const;

    String NewRenderer(String type, String dataset="");
    void DeleteRenderer(String name);

    int  Render(String imagePath, bool fast=false);
    void SetTimestep(int ts);
    int GetTimesteps() const;
    
    static void SetWaspMyBaseErrMsgFilePtrToSTDERR();
    
    String GetPythonWinName() const;

protected:
    void             loadAllParamsDatasets();
    void             getParamsDatasetInfo(String name, String *type, vector<String> *files);
    GUIStateParams * getGUIStateParams() const;
    AnimationParams *getAnimationParams() const;
    SettingsParams * getSettingsParams() const;
    String getWinName() const;
};
