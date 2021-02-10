#include "VComboBox.h"
#include <QStandardItemModel>

VComboBox::VComboBox(const std::vector<std::string> &values) : VHBoxWidget()
{
    _combo = new QComboBox;

    // Disable scroll wheel
    //
    _combo->setFocusPolicy(Qt::StrongFocus);
    _combo->installEventFilter(new MouseWheelWidgetAdjustmentGuard(_combo));

    layout()->addWidget(_combo);
    SetOptions(values);

    // We are forced to used SIGNAL/SLOT macros here because there are two
    // signatures for QComboBox::currentIndexChanged
    connect(_combo, SIGNAL(currentIndexChanged(QString)), this, SLOT(emitComboChanged(QString)));
}

// Stas thinks that we should have setValues and setValue instead of Update
//
void VComboBox::SetOptions(const std::vector<std::string> &values)
{
    _combo->blockSignals(true);
    _combo->clear();
    for (auto i : values) { _combo->addItem(QString::fromStdString(i)); }
    _combo->blockSignals(false);
}

void VComboBox::SetIndex(int index)
{
    if (index >= _combo->count()) return;

    _combo->blockSignals(true);
    _combo->setCurrentIndex(index);
    _combo->blockSignals(false);
}

void VComboBox::SetValue(const std::string &value)
{
    QString qValue = QString::fromStdString(value);
    int     index = _combo->findText(qValue);
    if (index >= 0) SetIndex(index);
}

void VComboBox::SetItemEnabled(int index, bool enabled)
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(_combo->model());
    assert(model);
    if (!model) return;

    model->item(index)->setEnabled(enabled);
}

int VComboBox::GetCurrentIndex() const { return _combo->currentIndex(); }

std::string VComboBox::GetValue() const { return _combo->currentText().toStdString(); }

int VComboBox::GetCount() const { return _combo->count(); }

void VComboBox::emitComboChanged(const QString &value)
{
    emit ValueChanged(value.toStdString());
    emit IndexChanged(_combo->currentIndex());
}
