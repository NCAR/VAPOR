#pragma once

#include <vector>
#include <cassert>
#include <QAction>

namespace VAPoR {
class ParamsBase;
}

// ******************************
//        ParamsMenuItem
// ******************************

//! \class ParamsMenuItem
//! Provides the same functionality as ParamsWidget except inside of a QMenu

class ParamsMenuItem : public QAction {
    Q_OBJECT

public:
    ParamsMenuItem(QObject *parent, const std::string &tag, const std::string &label = "");
    virtual void Update(VAPoR::ParamsBase *p) = 0;

protected:
    VAPoR::ParamsBase *_params = nullptr;
    std::string        _tag;
    std::string        _label;
};

// ******************************
//     ParamsCheckboxMenuItem
// ******************************

class ParamsCheckboxMenuItem : public ParamsMenuItem {
    Q_OBJECT

public:
    ParamsCheckboxMenuItem(QObject *parent, const std::string &tag, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);

private slots:
    void wasToggled(bool b);
};

// ******************************
//     ParamsDropdownMenuItem
// ******************************

class ParamsDropdownMenuItem : public ParamsMenuItem {
    Q_OBJECT

    std::vector<int>       _itemValues;
    std::vector<QAction *> _items;

public:
    ParamsDropdownMenuItem(QObject *parent, const std::string &tag, const std::vector<std::string> &items, const std::vector<int> &itemValues = {}, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);

private:
    void _selectIndex(int index);
    int  _getValueForIndex(int index) const;
    int  _getIndexForValue(int value) const;

private slots:
    void itemSelected();
};
