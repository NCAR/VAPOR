#include <QFileDialog>

#include "VaporWidgets.h"
#include "FileOperationChecker.h"
#include "ErrorReporter.h"

#include "ui_VPushButton.h"
#include "ui_VCheckBox.h"
#include "ui_VComboBox.h"
#include "ui_VPathSelector.h"

#include <iostream>
#include <cassert>

VPushButton::VPushButton(QWidget *parent, std::string labelText, std::string buttonText) : QWidget(parent), Ui_VPushButton()
{
    setupUi(this);

    SetLabelText(QString::fromStdString(labelText));
    SetButtonText(QString::fromStdString(buttonText));

    connect(_myButton, SIGNAL(pressed()), this, SLOT(_buttonPressed()));
}

void VPushButton::SetLabelText(const std::string text) { SetLabelText(QString::fromStdString(text)); }

void VPushButton::SetLabelText(const QString text) { _myLabel->setText(text); }

void VPushButton::SetButtonText(const std::string text) { SetButtonText(QString::fromStdString(text)); }

void VPushButton::SetButtonText(const QString text) { _myButton->setText(text); }

void VPushButton::_buttonPressed() { emit _pressed(); }

VComboBox::VComboBox(QWidget *parent, std::string labelText) : QWidget(parent), Ui_VComboBox()
{
    setupUi(this);

    SetLabelText(labelText);

    connect(_myCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(_userIndexChanged(int)));
}

void VComboBox::SetLabelText(std::string text) { _myLabel->setText(QString::fromStdString(text)); }

void VComboBox::_userIndexChanged(int index) { emit _indexChanged(index); }

int VComboBox::GetCurrentIndex() const { return _myCombo->currentIndex(); }

std::string VComboBox::GetCurrentText() const { return _myCombo->currentText().toStdString(); }

void VComboBox::AddOption(std::string option, int index) { _myCombo->insertItem(index, QString::fromStdString(option)); }

void VComboBox::RemoveOption(int index = 0) { _myCombo->removeItem(index); }

VCheckBox::VCheckBox(QWidget *parent, std::string labelText) : QWidget(parent), Ui_VCheckBox()
{
    setupUi(this);

    SetLabelText(labelText);

    connect(_myCheckbox, SIGNAL(stateChanged(int)), this, SLOT(_userClickedCheckbox()));
}

void VCheckBox::SetLabelText(std::string text) { _myLabel->setText(QString::fromStdString(text)); }

bool VCheckBox::GetCheckState() const
{
    if (_myCheckbox->checkState() == Qt::Checked)
        return true;
    else
        return false;
}

void VCheckBox::_userClickedCheckbox() { emit _checkboxClicked(); }

VPathSelector::VPathSelector(QWidget *parent, std::string labelText, std::string filePath) : QWidget(parent), Ui_VPathSelector()
{
    setupUi(this);

    SetLabelText(labelText);
    SetPath(filePath);

    connect(_myButton, SIGNAL(pressed()), this, SLOT(_openFileDialog()));
    connect(_myLineEdit, SIGNAL(returnPressed()), this, SLOT(_setFilePath()));
}

void VPathSelector::SetLabelText(std::string text) { _myLabel->setText(QString::fromStdString(text)); }

std::string VPathSelector::GetPath() const { return _filePath; }

void VPathSelector::SetPath(std::string path)
{
    _filePath = path;
    _myLineEdit->setText(QString::fromStdString(path));
    std::cout << "set path to " << _filePath << std::endl;
}

void VPathSelector::_openFileDialog()
{
    QString title = "Select file containing seed points";
    std::cout << "GetPath returns " << GetPath() << std::endl;
    QFileDialog fileDialog(this, title, QString::fromStdString(GetPath()));

    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    fileDialog.setAcceptMode(acceptMode);

    QFileDialog::FileMode fileMode = QFileDialog::ExistingFile;
    fileDialog.setFileMode(fileMode);
    if (fileDialog.exec() != QDialog::Accepted) return;

    QStringList files = fileDialog.selectedFiles();
    if (files.size() != 1) return;

    QString filePath = files[0];

    bool operable = FileOperationChecker::FileGoodToRead(filePath);
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
    QString filePath = _myLineEdit->text();
    bool    operable = FileOperationChecker::FileGoodToRead(filePath);
    if (!operable) {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
        SetPath(_filePath);
        return;
    }
    SetPath(filePath.toStdString());
}
