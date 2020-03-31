#include "PFileSelector.h"
#include "VPushButton.h"
#include "VLineEdit.h"
#include <QFileDialog>
#include <vapor/ParamsBase.h>
#include <vapor/FileUtils.h>
#include "SettingsParams.h"

PFileSelector::PFileSelector(const std::string &tag, const std::string &label) : PLineItem(tag, label, _pathTexbox = new VLineEdit, _button = new VPushButton("Select"))
{
    _pathTexbox->SetReadOnly(true);
    connect(_button, &VPushButton::ButtonClicked, this, &PFileSelector::buttonClicked);
}

void PFileSelector::updateGUI() const { _pathTexbox->SetValue(getParamsString()); }

PFileSelector *PFileSelector::SetFileTypeFilter(const std::string &filter)
{
    _fileTypeFilter = QString::fromStdString(filter);
    return this;
}

// PFileSelector *PFileSelector::UseDefaultPathSetting(const std::string &tag)
//{
//    _syncWithSettings = true;
//    _syncWithSettingsTag = tag;
//    return this;
//}

void PFileSelector::buttonClicked()
{
    string defaultPath;
    string selectedFile = getParamsString();

    if (_syncWithSettings) {
        // Too hardcoded in settings params to bother
    } else {
        if (Wasp::FileUtils::Exists(selectedFile))
            defaultPath = Wasp::FileUtils::Dirname(selectedFile);
        else
            defaultPath = Wasp::FileUtils::HomeDir();
    }

    QString qSelectedPath = selectPath(defaultPath);
    if (qSelectedPath.isNull()) return;

    setParamsString(qSelectedPath.toStdString());
}

bool PFileSelector::requireParamsMgr() const { return _syncWithSettings; }

QString PFileOpenSelector::selectPath(const std::string &defaultPath) const { return QFileDialog::getOpenFileName(nullptr, "Select a file", QString::fromStdString(defaultPath), _fileTypeFilter); }

QString PFileSaveSelector::selectPath(const std::string &defaultPath) const { return QFileDialog::getSaveFileName(nullptr, "Select save file", QString::fromStdString(defaultPath), _fileTypeFilter); }

QString PDirectorySelector::selectPath(const std::string &defaultPath) const { return QFileDialog::getExistingDirectory(nullptr, "Select a directory", QString::fromStdString(defaultPath)); }
