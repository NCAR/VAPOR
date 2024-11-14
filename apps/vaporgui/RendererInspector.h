#pragma once

#include "VRouter.h"

namespace VAPoR {
class ControlExec;
}
class RenderEventRouter;

class RendererInspector : public VRouter {
    VAPoR::ControlExec *_ce;
    std::map<std::string, std::pair<RenderEventRouter*, QWidget*>> _classToInspectorMap;

public:
    RendererInspector(VAPoR::ControlExec *ce);
    void Update();

    // Addresses legacy deps
    std::vector<RenderEventRouter*> GetRenderEventRouters() const;
};
