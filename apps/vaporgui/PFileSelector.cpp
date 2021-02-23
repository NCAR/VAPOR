#include "PFileSelector.h"
#include "VPushButton.h"
#include "VLineEdit_Deprecated.h"
#include <QFileDialog>
#include <vapor/ParamsBase.h>
#include <vapor/FileUtils.h>
#include "SettingsParams.h"

PFileSelector::PFileSelector(const std::string &tag, const std::string &label) : PLineItem(tag, label, _pathTexbox = new VLineEdit_Deprecated, _button = new VPushButton("Select"))
{
    _pathTexbox->SetReadOnly(true);
    _pathTexbox->setSizePolicy(QSizePolicy::Expanding, _pathTexbox->sizePolicy().verticalPolicy());
    connect(_button, &VPushButton::ButtonClicked, this, &PFileSelector::buttonClicked);
}

void PFileSelector::updateGUI() const
{
    const string path = getParamsString();
    _pathTexbox->SetValue(path);
    _pathTexbox->setToolTip(QString::fromStdString(path));
}

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
