#pragma once

#include "VContainer.h"
#include <QListWidget>

namespace VAPoR{
class ControlExec;
class RenderParams;
}
using namespace VAPoR;
using std::string;
class DisableDatasetClickEventFilter;
class NewRendererDialogManager;
class VHGroup;
class QToolButton;

class RendererList : public VContainer {
    Q_OBJECT
    QListWidget *_lw;
    QToolButton *_deleteToolButton;
    ControlExec *_ce;
    NewRendererDialogManager *_nrd;
    
    struct RendererMetadata {
        bool supports2D, supports3D;
        string description;
        string iconPath;
        string imagePath;
    };
    static bool AllowInspectDataset;
    static std::map<string, RendererMetadata> _rendererMetadata;
    static const int DatasetType = QListWidgetItem::UserType;
    static const int RendererType = QListWidgetItem::UserType+1;

    class RendererItem : public QListWidgetItem {
    public:
        const std::string Id;
        const std::string Class;
        const std::string Dataset;
        RendererItem(const std::string &instName, const std::string &className, const std::string &datasetName)
        : QListWidgetItem(nullptr, RendererType), Id(instName), Class(className), Dataset(datasetName) {}
    };
    class DatasetItem : public QListWidgetItem {
    public:
        const std::string Id;
        DatasetItem(const std::string &name)
        : QListWidgetItem(nullptr, DatasetType), Id(name) {}
    };

public:
    RendererList(ControlExec *ce);
    void Update();
    string getCurrentViz();
    string getClassName(string instName);
    void currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void inspectRenderer(RendererItem *item);
    void inspectDataset(DatasetItem *item);
    void showContextMenu(const QPoint& localPos);
    void deleteRenderer(string id);
    static std::vector<string> getHintVariables(RenderParams *rp);
    void renameRenderer(string inst);

protected:
    VHGroup *_toolbar;
    void resizeEvent(QResizeEvent *event);

    friend class DisableDatasetClickEventFilter;
};




