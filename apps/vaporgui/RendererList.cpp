#include "RendererList.h"

#include <QDebug>
#include <QScrollBar>
#include <QToolButton>
#include <QMenu>
#include <QInputDialog>
#include <vapor/STLUtils.h>
#include <vapor/DataStatus.h>
#include "VGroup.h"
#include "VVisibilityCheckbox.h"
#include "RenderEventRouter.h"
#include "NewRendererDialogManager.h"

std::map<string, RendererList::RendererMetadata> RendererList::_rendererMetadata;
bool RendererList::AllowInspectDataset = true;

// Fixes a bug in Qt.
class DisableDatasetClickEventFilter: public QObject {
    QListWidget *_w;
public:
    DisableDatasetClickEventFilter(QListWidget *w) : _w(w) {}
protected:
    bool eventFilter(QObject* object, QEvent* event)
    {
        if(event->type() == QEvent::MouseButtonRelease ||
           event->type() == QEvent::MouseButtonPress ||
           event->type() == QEvent::MouseButtonDblClick) {
            auto pos = _w->mapFromGlobal(QCursor::pos());
            auto *item = _w->itemAt(pos);
            if (item) {
                if (item->type() == RendererList::DatasetType) {
                    return true;
                }
            }
        }
        return QObject::eventFilter(object, event);
    }
};

class DisableDragFilter: public QObject {
    QListWidget *_w;
public:
    DisableDragFilter(QListWidget *w) : _w(w) {}
protected:
    bool eventFilter(QObject* object, QEvent* event)
    {
        // This event filter is ignored by disabled items.
//        if (event->type() == QEvent::MouseButtonPress) {
//            auto pos = _w->mapFromGlobal(QCursor::pos());
//            auto *item = _w->itemAt(pos);
//            if (item && !(item->flags() & Qt::ItemIsEnabled))
//                return true;
//        }
        if(event->type() == QEvent::MouseMove)
            return true;
        return QObject::eventFilter(object, event);
    }
};

RendererList::RendererList(ControlExec *ce) : VContainer(_lw = new QListWidget), _ce(ce)
{
    installEventFilter(new DisableDragFilter(_lw));
    connect(_lw, &QListWidget::currentItemChanged, this, &RendererList::currentItemChanged);
    _lw->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_lw, &QListWidget::customContextMenuRequested, this, &RendererList::showContextMenu);
    
    if (_rendererMetadata.empty()) {
        auto eventRouterNames = RenderEventRouterFactory::Instance()->GetFactoryNames();
        for (auto &rendererType : eventRouterNames) {
            auto router = std::unique_ptr<RenderEventRouter>(RenderEventRouterFactory::Instance()->CreateInstance(rendererType, nullptr, _ce));
            _rendererMetadata[rendererType] = {
                router->Supports2DVariables(),
                router->Supports3DVariables(),
                router->GetDescription(),
                router->GetSmallIconImagePath(),
                router->GetIconImagePath()
            };
        }
    }

    if (!AllowInspectDataset)
        _lw->viewport()->installEventFilter(new DisableDatasetClickEventFilter(_lw));

    
    _toolbar = new VHGroup;
    _toolbar->layout()->setSpacing(0);
    _toolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _toolbar->setParent(this);
    _toolbar->show();
    
    QToolButton *tb = new QToolButton;
    _deleteToolButton = tb;
    _toolbar->Add(tb);
    tb->setText("-");
    tb->setStyleSheet("QToolButton{font-size: 15pt;font-weight:bold; } QToolButton:enabled{color:#bf0206;}");
    connect(tb, &QToolButton::clicked, this, [=](){
        auto item = _lw->currentItem();
        if (item && item->type() == RendererType) {
            RendererItem *rItem = dynamic_cast<RendererItem*>(item);
            deleteRenderer(rItem->Id);
        }
    });

    _nrd = new NewRendererDialogManager(ce, this);
    tb = new QToolButton;
    _toolbar->Add(tb);
    tb->setText("+");
    connect(tb, &QToolButton::clicked, _nrd, &NewRendererDialogManager::Show);
    tb->setStyleSheet("QToolButton{font-size: 15pt;font-weight:bold;} QToolButton:enabled{color:#2b822b;}");
}

void RendererList::Update()
{
    auto paramsMgr = _ce->GetParamsMgr();
    GUIStateParams *guiParams = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    string currentViz = guiParams->GetActiveVizName();
    string activeRenderClass, activeRenderInst;
    guiParams->GetActiveRenderer(currentViz, activeRenderClass, activeRenderInst);
    string activeDataset = guiParams->GetActiveDataset();

    const auto allViz = paramsMgr->GetVisualizerNames();

    _lw->blockSignals(true);
    int scrollY = _lw->verticalScrollBar()->value();
    _lw->clear();

    auto datasets = guiParams->GetOpenDataSetNames();
    for (int i = 0; i < datasets.size(); i++) {
        auto dataset = datasets[i];
        if (i != 0) {
            QListWidgetItem *spacerItem = new QListWidgetItem();
            // These flags only sortof work
            spacerItem->setFlags(spacerItem->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable & ~Qt::ItemIsDragEnabled);
            _lw->addItem(spacerItem);
        }

        QListWidgetItem *datasetItem = new DatasetItem(dataset);
        datasetItem->setFlags(datasetItem->flags() & ~Qt::ItemIsDragEnabled);
        QLabel *datasetLabel = new QLabel(QString::fromStdString(dataset));
        datasetLabel->setStyleSheet("QLabel { font-weight: 500; }");
        if (!AllowInspectDataset)
            datasetItem->setFlags(datasetItem->flags() & ~Qt::ItemIsEnabled);
        _lw->addItem(datasetItem);
        _lw->setItemWidget(datasetItem, datasetLabel);
        if (AllowInspectDataset && dataset == activeDataset)
            _lw->setCurrentRow(_lw->count()-1);

        vector<string> rendererNames;
        paramsMgr->GetRenderParamNames(currentViz, dataset, rendererNames);
        
        for (auto rName : rendererNames) {
            const auto className = getClassName(rName);
            RenderParams *rp = _ce->GetRenderParams(currentViz, dataset, className, rName);
            
            VHGroup *line = new VHGroup;
            auto vc = new VVisibilityCheckbox;
            line->Add(vc);
            const auto vars = getHintVariables(rp);
            line->Add(new QLabel(QString::fromStdString(rp->GetValueString(rp->UserNameTag, rName) + (vars.empty() ? "" : " (" + STLUtils::Join(vars, ", ") + ")"))));

            vc->SetValue(rp->IsEnabled());
            
            connect(vc, &VVisibilityCheckbox::ValueChanged, this, [=](bool on){
                RenderParams *rp = _ce->GetRenderParams(currentViz, dataset, className, rName);
                if (rp) {
                    rp->SetEnabled(on);
                }
            });
            
            QListWidgetItem *item = new RendererItem(rName, className, dataset);
            item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
            _lw->addItem(item);
            _lw->setItemWidget(item, line);
            
            if (rName == activeRenderInst) {
                _lw->setCurrentRow(_lw->count()-1);
            }
        }
    }
    // _lw->scroll scrolls container, not the actual scroll area
    _lw->verticalScrollBar()->setValue(scrollY);
    _lw->blockSignals(false);

    _deleteToolButton->setEnabled(!activeRenderInst.empty());
}

string RendererList::getCurrentViz()
{
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    return guiParams->GetActiveVizName();
}

string RendererList::getClassName(string instName)
{
    auto classes = _ce->GetRenderClassNames(getCurrentViz());
    for (auto c : classes) {
        if (STLUtils::Contains(_ce->GetRenderInstances(getCurrentViz(), c), instName)) {
            return c;
        }
    }
    return "";
}

void RendererList::currentItemChanged(QListWidgetItem *item, QListWidgetItem *previous)
{
    if (item->type() == RendererType) {
        RendererItem *rItem = dynamic_cast<RendererItem*>(item);
        inspectRenderer(rItem);
    }
    else if (item->type() == DatasetType) {
        DatasetItem *dItem = dynamic_cast<DatasetItem*>(item);
        inspectDataset(dItem);
    }
}

void RendererList::inspectRenderer(RendererItem *rItem)
{
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    guiParams->SetActiveRenderer(guiParams->GetActiveVizName(), rItem->Class, rItem->Id);
}

void RendererList::inspectDataset(DatasetItem *item)
{
    GUIStateParams *guiParams = (GUIStateParams *)_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    guiParams->SetActiveDataset(item->Id);
}

void RendererList::showContextMenu(const QPoint& localPos)
{
    auto currentViz = getCurrentViz();
    
    QMenu _menu;
    QMenu *menu = &_menu;
    
    QListWidgetItem *item = _lw->itemAt(localPos);
    if (item && item->type() == RendererType) {
        string id, dataset, Class;
        { // Cannot capture rItem since after update caused by click may be recreated
            RendererItem *rItem = dynamic_cast<RendererItem *>(item);
            id = rItem->Id, dataset = rItem->Dataset, Class = rItem->Class;
        }
        RenderParams *rp = _ce->GetRenderParams(currentViz, dataset, Class, id);
        menu->addAction(rp->IsEnabled() ? "Hide" : "Show", [=](){rp->SetEnabled(!rp->IsEnabled());});
        menu->addAction("Delete", [=](){deleteRenderer(id);});
        menu->addAction("Rename", [=](){renameRenderer(id);});
        
        QMenu *duplicateMenu = menu->addMenu("Duplicate in");
        for (auto viz : _ce->GetVisualizerNames()) {
            duplicateMenu->addAction(QString::fromStdString(viz), [=](){
                auto newName = _ce->MakeRendererNameUnique(id);
                _ce->GetParamsMgr()->BeginSaveStateGroup("Duplicate Ren");
                auto newParams = _ce->GetRenderParams(currentViz, dataset, Class, id);
                _ce->ActivateRender(viz, dataset, newParams, newName, false);
                auto p = _ce->GetRenderParams(currentViz, dataset, Class, newName);
                if (!p->GetValueString(p->UserNameTag, "").empty())
                    p->SetValueString(p->UserNameTag, "", p->GetValueString(p->UserNameTag, "") + " (copy)");
                _ce->GetParamsMgr()->EndSaveStateGroup();
            });
        }
    }
    
    
    auto eventRouterNames = RenderEventRouterFactory::Instance()->GetFactoryNames();
    auto datasets = _ce->GetParamsMgr()->GetDataMgrNames();
    
    QMenu *newRendererMenu = menu->addMenu("New Renderer");
    
    for (const auto &dataset : datasets) {
        DataMgr *dm = _ce->GetDataStatus()->GetDataMgr(dataset);
        bool has2D = !dm->GetDataVarNames(2).empty();
        bool has3D = !dm->GetDataVarNames(3).empty();
        QMenu *datasetMenu = newRendererMenu;
        if (datasets.size() > 1)
            datasetMenu = newRendererMenu->addMenu(QString::fromStdString(dataset));
        for (const auto &rendererType : eventRouterNames) {
            QAction *action = datasetMenu->addAction(QString::fromStdString(rendererType), [=](){
                _ce->ActivateRender(currentViz, dataset, rendererType, _ce->MakeRendererNameUnique(rendererType), false);
            });
            const auto &meta = _rendererMetadata[rendererType];
            if (!(meta.supports2D && has2D) && !(meta.supports3D && has3D)) {
                action->setEnabled(false);
                action->setToolTip("This dataset has no 2D/3D variables");
            }
        }
    }
    
    menu->exec(_lw->viewport()->mapToGlobal(localPos));
}

void RendererList::deleteRenderer(std::string id)
{
    string rWin, rDataset, rClass;
    _ce->RenderLookup(id, rWin, rDataset, rClass);

    ParamsMgr *paramsMgr = _ce->GetParamsMgr();
    GUIStateParams *guiParams = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    paramsMgr->BeginSaveStateGroup(_ce->GetRemoveRendererUndoTag());
    _ce->RemoveRenderer(rWin, rDataset, rClass, id, false);
    guiParams->SetActiveRenderer(getCurrentViz(), "", "");

    vector<string> otherInsts;
    paramsMgr->GetRenderParamNames(rWin, otherInsts);

    // Need to set any other renderer otherwise other GUI code will break.
    for (auto &other : otherInsts) {
        if (other == id) continue;
        _ce->RenderLookup(other, rWin, rDataset, rClass);
        guiParams->SetActiveRenderer(guiParams->GetActiveVizName(), rClass, other);
        break;
    }
    paramsMgr->EndSaveStateGroup();
}

#include <vapor/ImageParams.h>
#include <vapor/FlowParams.h>
#include <vapor/BarbParams.h>
#include <vapor/ModelParams.h>
#include <vapor/ParticleParams.h>
std::vector<string> RendererList::getHintVariables(VAPoR::RenderParams *rp)
{
    auto helper = [](RenderParams *rp) -> vector<string> {
        if (dynamic_cast<FlowParams*>(rp) || dynamic_cast<BarbParams*>(rp) || dynamic_cast<ParticleParams*>(rp))
            return rp->GetFieldVariableNames();
        if (dynamic_cast<ImageParams*>(rp))
            return {rp->GetHeightVariableName()};
        if (dynamic_cast<ModelParams*>(rp))
            return {};
        return {rp->GetFirstVariableName()};
    };
    return STLUtils::Filter<string>(helper(rp), [](const string &v){return !v.empty();});
}

void RendererList::renameRenderer(std::string id)
{
    string rWin, rDataset, rClass;
    _ce->RenderLookup(id, rWin, rDataset, rClass);
    RenderParams *rp = _ce->GetRenderParams(rWin, rDataset, rClass, id);
    bool ok;
    string newName = QInputDialog::getText(this, "Rename Renderer", "", QLineEdit::Normal, QString::fromStdString(rp->GetValueString(rp->UserNameTag, id)), &ok).toStdString();
    if (ok)
        rp->SetValueString(rp->UserNameTag, "", newName);
}

void RendererList::resizeEvent(QResizeEvent *event)
{
    _toolbar->setFixedSize(_toolbar->sizeHint());
    auto p = size() - _toolbar->size();
    _toolbar->move(p.width(), p.height());
}
