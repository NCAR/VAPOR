#pragma once

#include "VContainer.h"

namespace VAPoR {
class ControlExec;
}
class RendererList;
class RendererInspector;
class DatasetInspector;
class VRouter;
class QSplitter;

class RenderersPanel : public VContainer {
    Q_OBJECT
    QSplitter *_splitter;
    VAPoR::ControlExec *_ce;
    RendererList *_renList;
    VRouter *_inspectorRouter;
    RendererInspector *_renInspector;
    DatasetInspector *_dataInspector;

public:
    RenderersPanel(VAPoR::ControlExec *ce);
    void Update();
};