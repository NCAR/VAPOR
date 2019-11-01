#include "ModelSubtabs.h"
#include "TFEditor.h"
#include "VSection.h"
#include "ParamsWidgets.h"
#include <vapor/ModelParams.h>

ModelVariablesSubtab::ModelVariablesSubtab(QWidget *parent)
{
    setupUi(this);
    _variablesWidget->hide();

    layout()->setContentsMargins(0, 10, 0, 0);
    layout()->addWidget(_modelSettings = new VSection("Model"));

    addPW(
        (new ParamsWidgetFile(VAPoR::ModelParams::FileTag, "Model/Scene File"))
            ->SetFileTypeFilter(
                "3D Models/Scenes (*.vms *.3d *.3ds *.3mf *.ac *.ac3d *.acc *.amj *.ase *.ask *.b3d *.blend *.bvh *.cms *.cob *.dae *.dxf *.enff *.fbx *.hmb *.ifc *.irr *.lwo *.lws *.lxo *.md2 *.md3 *.md5 *.mdc *.mdl *.mesh *.mot *.ms3d *.ndo *.nff *.obj *.off *.ogex *.ply *.pmx *.prj *.q3o *.q3s *.raw *.scn *.sib *.smd *.stp *.stl *.ter *.uc *.vta *.x *.x3d *.xml *.xgl *.zgl)"));
}

void ModelVariablesSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    for (auto w : _pw) w->Update(rParams);
}

void ModelVariablesSubtab::addPW(ParamsWidget *w)
{
    _modelSettings->layout()->addWidget(w);
    _pw.push_back(w);
}

ModelGeometrySubtab::ModelGeometrySubtab(QWidget *parent)
{
    setupUi(this);
    ((QVBoxLayout *)layout())->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
}
