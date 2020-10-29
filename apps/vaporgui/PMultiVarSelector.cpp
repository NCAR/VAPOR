#include "PMultiVarSelector.h"
#include <QListWidget>
#include <cassert>
#include <vapor/RenderParams.h>

using VAPoR::RenderParams;

PMultiVarSelector::PMultiVarSelector(std::string tag)
: PWidget(tag, _listWidget = new QListWidget)
{
    _listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    connect(_listWidget, &QListWidget::itemChanged, this, &PMultiVarSelector::itemChanged);
}

void PMultiVarSelector::updateGUI() const
{
    const auto nDims = getParams<RenderParams>()->GetRenderDim();
    const auto varNames = getDataMgr()->GetDataVarNames(nDims);
    
    _listWidget->blockSignals(true);
    _listWidget->clear();
    for (const auto &var : varNames)
        addVarToList(var);
    
    const auto selectedVars = getParams()->GetValueStringVec(getTag());
    for (const auto &var : selectedVars) {
        const auto items = _listWidget->findItems(QString::fromStdString(var), Qt::MatchExactly);
        QListWidgetItem *item;
        if (items.isEmpty())
            item = addVarToList(var);
        else
            item = items[0];
        item->setCheckState(Qt::Checked);
    }
    _listWidget->blockSignals(false);
}

QSize PMultiVarSelector::minimumSizeHint() const
{
    QSize s = PWidget::minimumSizeHint();
    int count = _listWidget->count();
    int rowHeight = _listWidget->sizeHintForRow(0);
    int height = count * rowHeight;
    s.setHeight(std::max(s.height(), std::min(s.height()*3, height)));
    return s;
}

QListWidgetItem *PMultiVarSelector::addVarToList(const std::string &var) const
{
    QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(var));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    _listWidget->addItem(item);
    return item;
}

void PMultiVarSelector::itemChanged(QListWidgetItem*)
{
    std::vector<std::string> selected;
    for (int i = 0; i < _listWidget->count(); i++) {
        QListWidgetItem *item = _listWidget->item(i);
        if (item->checkState() == Qt::Checked)
            selected.push_back(item->text().toStdString());
    }
    
    getParams()->SetValueStringVec(getTag(), "", selected);
}
