#pragma once

#include <QTabWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <string>

#include <QToolButton>

//! \class VSection
//! Represents a section/group to be used in the sidebar.
//! Provides a consistent layout which is not supposed to be changed
//! Provides a settings menu that is intended to provide extra options for the parameters that are
//! shown within this section, for example resetting to default values.
//!
//! Prefer the use of VSectionGroup when possible.

class VSection : public QTabWidget {
    Q_OBJECT

    class SettingsMenuButton;
    class ExpandSectionButton;
    class AbstractButton;

public:
    VSection(const std::string &title);
    QVBoxLayout *layout() const;
    void         setMenu(QMenu *menu);
    void         enableExpandedSection();
    std::string getTitle() const;

    void     setLayout(QLayout *layout) = delete;
    int      addTab(QWidget *page, const QString &label) = delete;
    QWidget *widget(int index) const = delete;
    void     setCornerWidget(QWidget *widget, Qt::Corner corner) = delete;
    QWidget *cornerWidget() const = delete;

private:
    QWidget *_tab() const;
    QString  _createStylesheet() const;

signals:
    void expandButtonClicked();
};

#include "AbstractWidgetGroup.h"
#include "VGroup.h"

//! \class VSectionGroup
//! \brief VSection that implements the standardized widget group interface. Use this one when possible.
//! \author Stas Jaroszynski

class VSectionGroup : public VSection, public WidgetGroupWrapper<VSectionGroup, QWidget, VGroup> {
public:
    VSectionGroup(const std::string &title, List children = {}) : VSection(title), WidgetGroupWrapper(new VGroup())
    {
        layout()->addWidget(_group);
        _group->AddM(children);
    }
};

class VSection::AbstractButton : public QToolButton {
    Q_OBJECT

protected:
    AbstractButton() {};
    void paintEvent(QPaintEvent *event);
    void configureButton();
    void setDarkOrLight();
    virtual void setIconDark(bool darkMode = true) = 0;
};

class VSection::SettingsMenuButton : public AbstractButton {
    Q_OBJECT

    void setIconDark(bool darkMode) override;

public:
    SettingsMenuButton();
};

class VSection::ExpandSectionButton: public AbstractButton {
    Q_OBJECT

    void setIconDark(bool darkMode) override;

public:
    ExpandSectionButton();
};
