#include <QFileDialog>

#include "VaporWidgets.h"
#include "FileOperationChecker.h"
#include "ErrorReporter.h"

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QSpacerItem>
#include <QHBoxLayout>

#include <iostream>
#include <cassert>

VaporWidget::VaporWidget(QWidget *parent, const std::string &labelText) : QWidget(parent)
{
    _layout = new QHBoxLayout();
    _layout->setContentsMargins(10, 0, 10, 0);
    setLayout(_layout);

    _label = new QLabel(this);
    _spacer = new QSpacerItem(10, 10, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    _layout->addWidget(_label);
    _layout->addItem(_spacer);

    SetLabelText(labelText);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

VaporWidget::VaporWidget(QWidget *parent, const QString &labelText) : VaporWidget(parent, labelText.toStdString()) {}

void VaporWidget::SetLabelText(const std::string &text) { _label->setText(QString::fromStdString(text)); }

void VaporWidget::SetLabelText(const QString &text) { _label->setText(text); }

VPushButton::VPushButton(QWidget *parent, const std::string &labelText, const std::string &buttonText) : VaporWidget(parent, labelText)
{
    _button = new QPushButton(this);
    _layout->addWidget(_button);

    SetLabelText(QString::fromStdString(labelText));
    SetButtonText(QString::fromStdString(buttonText));

    connect(_button, SIGNAL(pressed()), this, SLOT(_buttonPressed()));
}

void VPushButton::SetButtonText(const std::string &text) { SetButtonText(QString::fromStdString(text)); }

void VPushButton::SetButtonText(const QString &text) { _button->setText(text); }

void VPushButton::_buttonPressed() { emit _pressed(); }

VComboBox::VComboBox(QWidget *parent, std::string labelText) : VaporWidget(parent, labelText)
{
    _combo = new QComboBox(this);
    _layout->addWidget(_combo);

    connect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(_userIndexChanged(int)));
}

void VComboBox::_userIndexChanged(int index) { emit _indexChanged(index); }

int VComboBox::GetCurrentIndex() const { return _combo->currentIndex(); }

std::string VComboBox::GetCurrentText() const { return _combo->currentText().toStdString(); }

void VComboBox::AddOption(const std::string &option, int index) { _combo->insertItem(index, QString::fromStdString(option)); }

void VComboBox::RemoveOption(int index = 0) { _combo->removeItem(index); }

VCheckBox::VCheckBox(QWidget *parent, std::string labelText) : VaporWidget(parent, labelText)
{
    _checkbox = new QCheckBox("", this);
    _layout->addWidget(_checkbox);

    _layout->setContentsMargins(10, 0, 16, 0);

    connect(_checkbox, SIGNAL(stateChanged(int)), this, SLOT(_userClickedCheckbox()));
}

bool VCheckBox::GetCheckState() const
{
    if (_checkbox->checkState() == Qt::Checked)
        return true;
    else
        return false;
}

void VCheckBox::_userClickedCheckbox() { emit _checkboxClicked(); }

VPathSelector::VPathSelector(QWidget *parent, const std::string &labelText, const std::string &filePath, QFileDialog::FileMode fileMode) : VPushButton(parent, labelText)
{
    _fileMode = fileMode;

    _lineEdit = new QLineEdit(this);
    _layout->addWidget(_lineEdit);

    SetLabelText(labelText);
    SetPath(filePath);

    connect(_button, SIGNAL(pressed()), this, SLOT(_openFileDialog()));
    connect(_lineEdit, SIGNAL(returnPressed()), this, SLOT(_setFilePath()));
}

std::string VPathSelector::GetPath() const { return _filePath; }

void VPathSelector::SetPath(const std::string &path)
{
    _filePath = path;
    _lineEdit->setText(QString::fromStdString(path));
    std::cout << "set path to " << _filePath << std::endl;
}

void VPathSelector::_openFileDialog()
{
    QString title = "Select file containing seed points";
    std::cout << "GetPath returns " << GetPath() << std::endl;
    QFileDialog fileDialog(this, title, QString::fromStdString(GetPath()));

    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    fileDialog.setAcceptMode(acceptMode);

    fileDialog.setFileMode(_fileMode);

    if (fileDialog.exec() != QDialog::Accepted) return;

    QStringList files = fileDialog.selectedFiles();
    if (files.size() != 1) return;

    QString filePath = files[0];

    bool operable;
    if (_fileMode == QFileDialog::FileMode::ExistingFile) operable = FileOperationChecker::FileGoodToRead(filePath);
    if (_fileMode == QFileDialog::FileMode::Directory) operable = FileOperationChecker::DirectoryGoodToRead(filePath);

    if (!operable) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        SetPath(_filePath);
        return;
    }

    SetPath(filePath.toStdString());
}

void VPathSelector::_setFilePath()
{
    std::cout << "setFilePath" << std::endl;
    QString filePath = _lineEdit->text();

    bool operable = FileOperationChecker::FileGoodToRead(filePath);
    if (!operable) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        SetPath(_filePath);
        return;
    }
    SetPath(filePath.toStdString());
}
