#ifndef VAPORWIDGETS_H 
#define VAPORWIDGETS_H 

#include <QWidget>
#include <QDoubleValidator>

#include "Combo.h"
#include "ui_VPushButton.h"
#include "ui_VComboBox.h"
#include "ui_VCheckBox.h"
#include "ui_VPathSelector.h"

namespace Ui {
class QSliderEdit;
}


class VPushButton: public QWidget, public Ui_VPushButton
{
    Q_OBJECT

public:
    VPushButton(
        QWidget* parent, 
        std::string labelText = "Label",
        std::string buttonText = "Button"
    );

    void SetLabelText(  const std::string text );
    void SetLabelText(  const QString text );
    void SetButtonText( const std::string text );
    void SetButtonText( const QString text );

signals:
    void _pressed();

private slots:
    void _buttonPressed();
};



class VComboBox : public QWidget, public Ui_VComboBox
{
    Q_OBJECT

public:
    VComboBox(
        QWidget* parent,
        std::string labelText = "Label"
    );
    void        SetLabelText(  const std::string text );
    int         GetCurrentIndex() const;
    std::string GetCurrentText() const;
    void        AddOption( std::string option, int index=0 );
    void        RemoveOption( int index );

private slots:
    void _userIndexChanged(int index);

signals:
    void _indexChanged(int index);
};



class VCheckBox : public QWidget, public Ui_VCheckBox
{
    Q_OBJECT

public:
    VCheckBox(
        QWidget* parent,
        std::string labelText = "Label"
    );
    void SetLabelText( std::string text );
    bool GetCheckState() const;

private slots:
    void _userClickedCheckbox();

signals:
    void _checkboxClicked();
};

class VPathSelector : public QWidget, public Ui_VPathSelector
{
    Q_OBJECT

public:
    VPathSelector(
        QWidget* parent,
        std::string labelText = "Label",
        std::string filePath = QDir::homePath().toStdString()
    );
    void SetLabelText( std::string text );
    std::string GetPath() const;
    void SetPath( std::string defaultPath);

private slots:
    void _openFileDialog();
    void _setFilePath();

signals:
    void _pathChanged();

private:
    std::string _filePath;
};


#endif // VAPORWIDGETS_H
