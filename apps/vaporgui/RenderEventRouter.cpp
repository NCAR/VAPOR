#include <QFileDialog>
#include <vapor/ResourcePath.h>
#include <vapor/DataMgrUtils.h>
#include "RenderEventRouter.h"
#include <vapor/Renderer.h>
#include <vapor/DataStatus.h>
#include <vapor/NavigationUtils.h>
#include <vapor/ControlExecutive.h>

using namespace VAPoR;

RenderParams *RenderEventRouter::GetActiveParams() const
{
    VAssert(!_instName.empty());

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    string winName, dataSetName, paramsType;
    bool   status = paramsMgr->RenderParamsLookup(_instName, winName, dataSetName, paramsType);
    if (!status)
        return nullptr;

    string renderType = RendererFactory::Instance()->GetRenderClassFromParamsClass(paramsType);

    return (_controlExec->GetRenderParams(winName, dataSetName, renderType, _instName));
}

DataMgr *RenderEventRouter::GetActiveDataMgr() const
{
    VAssert(!_instName.empty());

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    string winName, dataSetName, paramsType;

    bool status = paramsMgr->RenderParamsLookup(_instName, winName, dataSetName, paramsType);
    VAssert(status);

    DataStatus *dataStatus = _controlExec->GetDataStatus();
    DataMgr *   dataMgr = dataStatus->GetDataMgr(dataSetName);
    VAssert(dataMgr);

    return (dataMgr);
}

string RenderEventRouter::GetSmallIconImagePath() const
{
    string imageName = _getSmallIconImagePath();
    if (imageName.empty()) return (imageName);

    return (GetSharePath("images/" + imageName));
}

string RenderEventRouter::GetIconImagePath() const
{
    string imageName = _getIconImagePath();
    if (imageName.empty()) return (imageName);

    return (GetSharePath("images/" + imageName));
}


//////////////////////////////////////////////////////////////////////////
//
// RenderEventRouterFactory Class
//
/////////////////////////////////////////////////////////////////////////


RenderEventRouter *RenderEventRouterFactory::CreateInstance(string className, QWidget *parent, VAPoR::ControlExec *ce)
{
    RenderEventRouter *instance = NULL;

    auto it = _factoryFunctionRegistry.find(className);
    if (it != _factoryFunctionRegistry.end()) instance = it->second(parent, ce);

    return instance;
}

vector<string> RenderEventRouterFactory::GetFactoryNames() const
{
    vector<string> names;
    map<string, function<RenderEventRouter *(QWidget *, VAPoR::ControlExec *)>>::const_iterator itr;

    for (itr = _factoryFunctionRegistry.begin(); itr != _factoryFunctionRegistry.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}
