#pragma once

#include "VContainer.h"
#include "Updatable.h"
#include "VaporFwd.h"

class RendererList;
class RendererInspector;
class DatasetInspector;
class VRouter;
class QSplitter;

class RenderersPanel : public VContainer, public Updatable {
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