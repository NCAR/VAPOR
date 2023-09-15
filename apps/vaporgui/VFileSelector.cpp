#include "VFileSelector.h"

#include <iostream>

#include "FileOperationChecker.h"
#include "ErrorReporter.h"

#include "VPushButton.h"
#include "VLineEdit_Deprecated.h"

VFileSelector::VFileSelector(const std::string &buttonText, const std::string &defaultPath, const std::string &filter = "") : VHBoxWidget(), _filePath(defaultPath), _filter(filter)
{
    _pushButton = new VPushButton(buttonText);
    _lineEdit = new VLineEdit_Deprecated(defaultPath);
    layout()->addWidget(_pushButton);
    layout()->addWidget(_lineEdit);

    if (_filePath.empty()) _filePath = QDir::homePath().toStdString();

    connect(_pushButton, &VPushButton::ButtonClicked, this, &VFileSelector::OpenFileDialog);
    connect(_lineEdit, SIGNAL(ValueChanged(std::string)), this, SLOT(SetPathFromLineEdit(std::string)));
}

std::string VFileSelector::GetValue() const { return _filePath; }

bool VFileSelector::SetValue(const std::string &file)
{
    bool success = false;

    if (file == _filePath)    // Do nothing, if nothing is changed
        return success;

    if (_isFileOperable(file)) {
        _filePath = file;
        success = true;
    } else {
        MSG_ERR(FileOperationChecker::GetLastErrorMessage().toStdString());
    }

    _lineEdit->SetValue(_filePath);
    _lineEdit->setToolTip(QString::fromStdString(_filePath));
    return success;
}

void VFileSelector::OpenFileDialog()
{
    std::string file = _launchFileDialog();
    if (SetValue(file)) { emit ValueChanged(_filePath); }
}

void VFileSelector::SetPathFromLineEdit(const std::string &file)
{
    if (SetValue(file)) emit ValueChanged(_filePath);
}

void VFileSelector::HideLineEdit(bool hide)
{
    if (hide)
        _lineEdit->hide();
    else
        _lineEdit->show();
}

//
//
//
VFileReader::VFileReader(const std::string &buttonText, const std::string &defaultPath, const std::string &filter) : VFileSelector(buttonText, defaultPath, filter) {}

std::string VFileReader::_launchFileDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select Directory/File", QString::fromStdString(_filePath), QString::fromStdString(_filter));
    return fileName.toStdString();
}

bool VFileReader::_isFileOperable(const std::string &filePath) const
{
    bool operable = false;
    operable = FileOperationChecker::FileGoodToRead(QString::fromStdString(filePath));
    return operable;
}

//
//
//
VFileWriter::VFileWriter(const std::string &buttonText, const std::string &defaultPath, const std::string &filter) : VFileSelector(buttonText, defaultPath, filter) {}

bool VFileWriter::_isFileOperable(const std::string &filePath) const
{
    bool operable = false;
    operable = FileOperationChecker::FileGoodToWrite(QString::fromStdString(filePath));
    return operable;
}

std::string VFileWriter::_launchFileDialog()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Select Directory/File", QString::fromStdString(_filePath), QString::fromStdString(_filter));
    return fileName.toStdString();
}

VDirSelector::VDirSelector(const std::string &buttonText, const std::string &defaultPath) : VFileSelector(buttonText, defaultPath) {}

std::string VDirSelector::_launchFileDialog()
{
    QString fileName = QFileDialog::getExistingDirectory(this, "Select Directory/File", QString::fromStdString(_filePath));
    return fileName.toStdString();
}

bool VDirSelector::_isFileOperable(const std::string &filePath) const
{
    bool operable = false;
    operable = FileOperationChecker::DirectoryGoodToRead(QString::fromStdString(filePath));
    return operable;
}
