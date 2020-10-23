#pragma once

#include <string>

#include <QWidgetAction>

class VIntSpinBox;
class VCheckBox;
class VStringLineEdit;
class VIntLineEdit;
class VDoubleLineEdit;

#include "VHBoxWidget.h"

class VLineAction : public QWidgetAction {
    Q_OBJECT

public:
    VLineAction(const std::string &title, VHBoxWidget *container);
};

//!
//! \class VSpinBoxAction
//! \ingroup Public_GUI
//! \brief A menu item represented by a VLabel and VSpinBox, wrapped in a VLineItem
//! selection tab in any renderer EventRouter class

//! A QWidgetAction that represents itself as a VLineItem, which contains
//! a VLabel and a VSpinBox.  VWidget's standard SetValue()
//! function can be used to retrieve and update the current value.  Used
//! within QMenu classes.

class VSpinBoxAction : public VLineAction {
    Q_OBJECT

public:
    VSpinBoxAction(const std::string &title, int value);

    //! Set the current integer value held by the VSpinBox
    void SetValue(int value);

private:
    VIntSpinBox *_spinBox;

private slots:

    //! Respond to a change in the VSpinBox
    void _spinBoxChanged(int value);

signals:
    void editingFinished(int);
};

//!
//! \class VCheckBoxAction
//! \ingroup Public_GUI
//! \brief A menu item represented by a VLabel and VCheckBox, wrapped in a VLineItem

//! A QWidgetAction that represents itself as a VLineItem, which contains
//! a VLabel and a VCheckBox.  VWidget's standard SetValue()
//! function can be used to retrieve and update the current value.  Used
//! within QMenu classes.

class VCheckBoxAction : public VLineAction {
    Q_OBJECT

public:
    VCheckBoxAction(const std::string &title, bool value);

    //! Set the checkstate of the VCheckBox (0=unchecked, 1=checked)
    void SetValue(bool value);

private:
    VCheckBox *_checkBox;

private slots:

    //! Respond to a change in the checkstate of the VCheckBox
    void _checkBoxChanged(bool value);

signals:
    void clicked(bool);
};

//!
//! \class VStringLineEditAction
//! \ingroup Public_GUI
//! \brief A menu item represented by a VLabel and VStringLineEdit, wrapped in a VLineItem

//! A QWidgetAction that represents itself as a VLineItem, which contains
//! a VLabel and a VStringLineEditAction.  VWidget's standard SetValue()
//! function can be used to retrieve and update the current value.  Used
//! within QMenu classes.

class VStringLineEditAction : public VLineAction {
    Q_OBJECT

public:
    VStringLineEditAction(const std::string &title, std::string value);

    //! Set the string value held by the VStringLineEdit
    void SetValue(const std::string &value);

private:
    VStringLineEdit *_lineEdit;

private slots:

    //! Respond to a change made (editingFinished, or returnPressed) in the V*LineEdit
    void _lineEditChanged(int value);

signals:
    void ValueChanged(int);
};

//!
//! \class VIntLineEditAction
//! \ingroup Public_GUI
//! \brief A menu item represented by a VLabel and VIntLineEdit, wrapped in a VLineItem

//! A QWidgetAction that represents itself as a VLineItem, which contains
//! a VLabel and a VIntLineEditAction.  VWidget's standard SetValue()
//! function can be used to retrieve and update the current value.  Used
//! within QMenu classes.

class VIntLineEditAction : public VLineAction {
    Q_OBJECT

public:
    VIntLineEditAction(const std::string &title, int value);

    //! Set the numeric value held by the VIntLineEdit
    void SetValue(int value);

private:
    VIntLineEdit *_lineEdit;

private slots:

    //! @copydoc VStringLineEditAction::SetValue()
    void _lineEditChanged(int value);

signals:
    void ValueChanged(int);
};

//!
//! \class VIntLineEditAction
//! \ingroup Public_GUI
//! \brief A menu item represented by a VLabel and VIntLineEdit, wrapped in a VLineItem

//! A QWidgetAction that represents itself as a VLineItem, which contains
//! a VLabel and a VDoubleLineEditAction.  VWidget's standard SetValue()
//! function can be used to retrieve and update the current value.  Used
//! within QMenu classes.

class VDoubleLineEditAction : public VLineAction {
    Q_OBJECT

public:
    VDoubleLineEditAction(const std::string &title, double value);

    //! @copydoc VIntLineEditAction::SetValue()
    void SetValue(double value);

private:
    VDoubleLineEdit *_lineEdit;

private slots:

    //! @copydoc VIntLineEditAction::_lineEditChanged()
    void _lineEditChanged(double value);

signals:
    void ValueChanged(double);
};
