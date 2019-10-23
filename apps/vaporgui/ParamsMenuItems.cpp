#include "ParamsMenuItems.h"
#include <QMenu>
#include <vapor/ParamsBase.h>

using namespace VAPoR;

// ******************************
//        ParamsMenuItem
// ******************************

ParamsMenuItem::ParamsMenuItem(QObject *parent, const std::string &tag, const std::string &label) : QAction(parent)
{
    assert(!tag.empty());
    _tag = tag;

    if (label.empty())
        _label = tag;
    else
        _label = label;

    setText(QString::fromStdString(_label));
}

// ******************************
//     ParamsCheckboxMenuItem
// ******************************

ParamsCheckboxMenuItem::ParamsCheckboxMenuItem(QObject *parent, const std::string &tag, const std::string &label) : ParamsMenuItem(parent, tag, label)
{
    this->setCheckable(true);
    connect(this, SIGNAL(toggled(bool)), this, SLOT(wasToggled(bool)));
}

void ParamsCheckboxMenuItem::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    setChecked(p->GetValueLong(_tag, 0));
}

void ParamsCheckboxMenuItem::wasToggled(bool b)
{
    if (_params) _params->SetValueLong(_tag, _tag, b);
}

// ******************************
//     ParamsDropdownMenuItem
// ******************************

ParamsDropdownMenuItem::ParamsDropdownMenuItem(QObject *parent, const std::string &tag, const std::vector<std::string> &items, const std::vector<int> &itemValues, const std::string &labelText)
: ParamsMenuItem(parent, tag, labelText)
{
    QMenu *menu = new QMenu;

    for (string item : items) {
        QAction *action = menu->addAction(QString::fromStdString(item), this, SLOT(itemSelected()));
        action->setCheckable(true);
        _items.push_back(action);
    }

    if (!itemValues.empty()) {
        assert(itemValues.size() == items.size());
        _itemValues = itemValues;
    }

    this->setMenu(menu);
}

void ParamsDropdownMenuItem::Update(VAPoR::ParamsBase *p)
{
    _params = p;

    int value = p->GetValueLong(_tag, 0);
    _selectIndex(_getIndexForValue(value));
}

void ParamsDropdownMenuItem::itemSelected()
{
    if (!_params) return;

    int index = -1;
    for (int i = 0; i < _items.size(); i++) {
        if (_items[i] == sender()) {
            index = i;
            break;
        }
    }

    VAssert(index != -1);

    _selectIndex(index);
    _params->SetValueLong(_tag, _tag, _getValueForIndex(index));
}

void ParamsDropdownMenuItem::_selectIndex(int index)
{
    assert(index >= 0 && index < _items.size());
    if (!(index >= 0 && index < _items.size())) return;

    for (int i = 0; i < _items.size(); i++) _items[i]->setChecked(false);
    _items[index]->setChecked(true);
}

int ParamsDropdownMenuItem::_getValueForIndex(int index) const
{
    if (_itemValues.empty()) return index;

    if (index >= _itemValues.size() || index < 0) VAssert("Invalid menu index");

    return _itemValues[index];
}

int ParamsDropdownMenuItem::_getIndexForValue(int value) const
{
    if (_itemValues.empty()) return value;

    const int N = _itemValues.size();
    for (int i = 0; i < N; i++)
        if (_itemValues[i] == value) return i;

    VAssert("Invalid params value");
    return -1;
}
