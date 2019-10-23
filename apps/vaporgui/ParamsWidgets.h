#pragma once

#include <vector>
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <vapor/ParamsBase.h>
#include <cassert>
#include <QSpacerItem>

//! \class ParamsWidget
//! Provedes a GUI element that is synced with the Params database
//! The parameter tag that it is linked to is passed in the constructor and the
//! relevant params node is set in the update method.

class ParamsWidget : public QWidget {
    Q_OBJECT

public:
    //! \param[in] label will be set to tag by default
    ParamsWidget(const std::string &tag, const std::string &label = "");
    virtual void Update(VAPoR::ParamsBase *p) = 0;

protected:
    VAPoR::ParamsBase *_params = nullptr;
    std::string        _tag;
    std::string        _label;
    QSpacerItem *      _spacer;
};

class ParamsWidgetCheckbox : public ParamsWidget {
    Q_OBJECT

    QCheckBox *_checkBox = nullptr;

public:
    ParamsWidgetCheckbox(const std::string &tag, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);

private slots:
    void checkbox_clicked(bool checked);
};

class ParamsWidgetNumber : public ParamsWidget {
    Q_OBJECT

    QLineEdit *_lineEdit = nullptr;

public:
    ParamsWidgetNumber(const std::string &tag, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);

    ParamsWidgetNumber *SetRange(int min, int max);

private slots:
    void valueChangedSlot();
};

class ParamsWidgetFloat : public ParamsWidget {
    Q_OBJECT

    QLineEdit *_lineEdit = nullptr;

public:
    ParamsWidgetFloat(const std::string &tag, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);

    ParamsWidgetFloat *SetRange(float min, float max);

private slots:
    void valueChangedSlot();
};

class ParamsWidgetDropdown : public ParamsWidget {
    Q_OBJECT

    QComboBox *      _box = nullptr;
    std::vector<int> _itemValues;

public:
    ParamsWidgetDropdown(const std::string &tag, const std::vector<std::string> &items, const std::vector<int> &itemValues = {}, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);

private:
    int getValueForIndex(int index) const;
    int getIndexForValue(int value) const;

private slots:
    void indexChangedSlot(int index);
};

class QColorWidget;
class ParamsWidgetColor : public ParamsWidget {
    Q_OBJECT

    QColorWidget *_color = nullptr;

public:
    ParamsWidgetColor(const std::string &tag, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);

    static QColor              VectorToQColor(const std::vector<double> &v);
    static std::vector<double> QColorToVector(const QColor &c);

private slots:
    void colorChanged(QColor color);
};

class ParamsWidgetFile : public ParamsWidget {
    Q_OBJECT

    QPushButton *_button = nullptr;
    QLineEdit *  _pathTexbox;
    QString      _fileTypeFilter = "All Files (*)";

public:
    ParamsWidgetFile(const std::string &tag, const std::string &label = "");
    void              Update(VAPoR::ParamsBase *p);
    ParamsWidgetFile *SetFileTypeFilter(const std::string &filter);

private slots:
    void _buttonClicked();
};

/*
 Placeholder. Does not currently work due to layout issues
class ParamsWidgetTextLabel : public ParamsWidget {
    Q_OBJECT

    QLabel *_textLabel = nullptr;

public:
    ParamsWidgetTextLabel(const std::string &tag, const std::string &label = "");
    void Update(VAPoR::ParamsBase *p);
};
 */

class ParamsWidgetGroup : public QGroupBox {
    Q_OBJECT

    bool fontUpdated = false;

public:
    ParamsWidgetGroup(const std::string &title);

protected:
    void changeEvent(QEvent *event);
};

class ParamsWidgetTabGroup : public QTabWidget {
    Q_OBJECT

    QWidget *                   _tab() const;
    std::vector<ParamsWidget *> _widgets;

public:
    ParamsWidgetTabGroup(const std::string &title);
    void Update(VAPoR::ParamsBase *p);
    void Add(ParamsWidget *widget);
};
