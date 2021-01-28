#include "ModelEventRouter.h"
#include "vapor/ModelParams.h"
#include "PWidgets.h"

using namespace VAPoR;

static RenderEventRouterRegistrar<ModelEventRouter> registrar(ModelEventRouter::GetClassType());

ModelEventRouter::ModelEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, ModelParams::GetClassType())
{
    // clang-format off

    AddSubtab("Model", new PGroup({
        new PSection("Model File", {
            (new PFileOpenSelector(ModelParams::FileTag, "Model/Scene File"))
                ->SetFileTypeFilter("3D Models/Scenes (*.vms *.3d *.3ds *.3mf *.ac *.ac3d *.acc *.amj *.ase *.ask *.b3d *.blend *.bvh *.cms *.cob *.dae *.dxf *.enff *.fbx *.hmb *.ifc *.irr *.lwo *.lws *.lxo *.md2 *.md3 *.md5 *.mdc *.mdl *.mesh *.mot *.ms3d *.ndo *.nff *.obj *.off *.ogex *.ply *.pmx *.prj *.q3o *.q3s *.raw *.scn *.sib *.smd *.stp *.stl *.ter *.uc *.vta *.x *.x3d *.xml *.xgl *.zgl)")
        }),
        new PRendererTransformWidget
    }));

    // clang-format on
}

string ModelEventRouter::_getDescription() const
{
    return ("Allows the import of 3D model files as well as more complex scenes that "
            "can be modified per timestep by using .vms files");
}
