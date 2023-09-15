#include <string>
#include <sstream>
#include <iomanip>

#include <QString>
#include <QLineEdit>

#include <VStringLineEdit.h>

VStringLineEdit::VStringLineEdit(std::string value) : VHBoxWidget(), _lineEdit(new QLineEdit), _strValue(value)
{
    layout()->addWidget(_lineEdit);
    connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(_valueChanged()));

    _lineEdit->setContextMenuPolicy(Qt::DefaultContextMenu);
    _lineEdit->setText(QString::fromStdString(value));
    _lineEdit->setToolTip(QString::fromStdString(value));
}

void VStringLineEdit::SetValueString(std::string value)
{
    _strValue = value;
    _lineEdit->setText(QString::fromStdString(_strValue));
    if (_autoTooltip) _lineEdit->setToolTip(QString::fromStdString(_strValue));
    _lineEdit->setCursorPosition(0);
}

std::string VStringLineEdit::GetValueString() const { return _strValue; }

void VStringLineEdit::RemoveContextMenu()
{
    _lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    setContextMenuPolicy(Qt::NoContextMenu);
}

void VStringLineEdit::SetCustomContextMenu()
{
    _lineEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_lineEdit, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(_showMenu(const QPoint &)));
}

void VStringLineEdit::SetAutoTooltip(bool on)
{
    _autoTooltip = on;
    if (on)
        _lineEdit->setToolTip(_lineEdit->text());
    else
        _lineEdit->setToolTip("");
}

void VStringLineEdit::_valueChanged()
{
    std::string value = _lineEdit->text().toStdString();
    if (value != _strValue) {
        _strValue = value;
        emit ValueChanged(_strValue);
    }
}

std::string VStringLineEdit::_getText() const
{
    std::string value = _lineEdit->text().toStdString();
    return value;
}
