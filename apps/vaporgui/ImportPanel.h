#pragma once

#include "VContainer.h"
#include "Updatable.h"
#include "VaporFwd.h"
//#include "PDatasetImportTypeSelector.h"
#include "PImportData.h"

class RendererList;
class DatasetImporter;
class RendererInspector;
class DatasetInspector;
class VRouter;
class QSplitter;
//class PDatasetImportTypeSelector;
class PImportData;
class PGroup;
class VGroup;
class PSection;
class VSection;
class VSectionGroup;

//class ImportPanel : public VContainer, public Updatable {
class ImportPanel : public QWidget, public Updatable {
    Q_OBJECT
    QSplitter *_splitter;

    VAPoR::ControlExec *_ce;

    //PGroup* _group;
    VGroup* _group;
    //VSection* _section;
    VSectionGroup* _section;

    RendererList *_renList;
    VRouter *_inspectorRouter;

    //PDatasetImportTypeSelector *_importer;
    //PImportDataSection *_importer;
    PImportData *_importer;
    RendererInspector *_renInspector;
    DatasetInspector *_dataInspector;

public:
    ImportPanel(VAPoR::ControlExec *ce);
    void Update() override;
};
