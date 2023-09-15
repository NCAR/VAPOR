#include "NewRendererDialogManager.h"
#include <vapor/ControlExecutive.h>
#include <QApplication>
#include "ErrorReporter.h"

NewRendererDialogManager::NewRendererDialogManager(VAPoR::ControlExec *ce, QWidget *parent)
: _ce(ce), _nrd(ConstructNewRendererDialog(ce)){
    _nrd->setVisible(false);
    _nrd->setParent(parent);
    _nrd->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    connect(_nrd, &NewRendererDialog::accepted, this, [this](){RenderHolder::_newRendererDialogAccepted(_ce, _nrd);});
}

void NewRendererDialogManager::Show()
{
    _nrd->InitializeDataSources(_ce->GetDataStatus());
    _nrd->open();
}

NewRendererDialog* NewRendererDialogManager::ConstructNewRendererDialog(vector<RenderEventRouter*> routers)
{
    vector<string>    widgetNames;
    vector<string>    descriptions;
    vector<string>    iconPaths;
    vector<string>    smallIconPaths;
    vector<bool>      dim2dSupport;
    vector<bool>      dim3dSupport;
    vector<bool>      particleSupport;

    for (int i = 0; i < routers.size(); i++) {
        RenderEventRouter *re = routers[i];

        widgetNames.push_back(re->GetType());
        descriptions.push_back(re->GetDescription());
        iconPaths.push_back(re->GetIconImagePath());
        smallIconPaths.push_back(re->GetSmallIconImagePath());

        dim2dSupport.push_back(re->Supports2DVariables());
        dim3dSupport.push_back(re->Supports3DVariables());
        particleSupport.push_back(re->SupportsParticleVariables());
    }

    return new NewRendererDialog(nullptr, widgetNames, descriptions, iconPaths, smallIconPaths, dim2dSupport, dim3dSupport, particleSupport);
}

NewRendererDialog *NewRendererDialogManager::ConstructNewRendererDialog(ControlExec *ce)
{
    vector<string> rendererNames = RenderEventRouterFactory::Instance()->GetFactoryNames();
    vector<std::unique_ptr<RenderEventRouter>> routersOwner;
    vector<RenderEventRouter*> routers;

    for (auto name : rendererNames) {
        RenderEventRouter *re = RenderEventRouterFactory::Instance()->CreateInstance(name, NULL, ce);
        routersOwner.push_back(std::unique_ptr<RenderEventRouter>(re));
        routers.push_back(re);
    }

    return ConstructNewRendererDialog(routers);
}