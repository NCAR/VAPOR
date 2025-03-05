#include "PFileSelector.h"
#include "VPushButton.h"
#include "VLineEdit_Deprecated.h"
#include <QFileDialog>
#include <vapor/ParamsBase.h>
#include <vapor/FileUtils.h>
#include <vapor/SettingsParams.h>

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

void PFileSelector::buttonClicked()
{
    std::string defaultPath = getDefaultPath();
    QString qSelectedPath = selectPath(defaultPath);
    if (qSelectedPath.isNull()) return;

    setParamsString(qSelectedPath.toStdString());
}

bool PFileSelector::requireParamsMgr() const { return _syncWithSettings; }

std::string PFileSelector::getDefaultPath() const {
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
    return defaultPath;
}

void PFilesOpenSelector::buttonClicked() {
    std::string defaultPath = getDefaultPath();
    QStringList qSelectedPaths = selectPaths(defaultPath);
    if (qSelectedPaths.isEmpty()) return;

    std::vector<std::string> paths;
    paths.reserve(qSelectedPaths.size());
    for (const QString& qPath : qSelectedPaths) paths.push_back(qPath.toStdString());
    getParams()->SetValueStringVec(getTag(), "", paths);
    emit filesSelected();
}

QStringList PFilesOpenSelector::selectPaths(const std::string &defaultPath) const { return QFileDialog::getOpenFileNames(nullptr, "Select file(s)", QString::fromStdString(defaultPath), _fileTypeFilter); }

QString PFilesOpenSelector::selectPath(const std::string &defaultPath) const { return QFileDialog::getOpenFileName(nullptr, "Select a file", QString::fromStdString(defaultPath), _fileTypeFilter); }

QString PFileOpenSelector::selectPath(const std::string &defaultPath) const { return QFileDialog::getOpenFileName(nullptr, "Select a file", QString::fromStdString(defaultPath), _fileTypeFilter); }

QString PFileSaveSelector::selectPath(const std::string &defaultPath) const { return QFileDialog::getSaveFileName(nullptr, "Select save file", QString::fromStdString(defaultPath), _fileTypeFilter); }

QString PDirectorySelector::selectPath(const std::string &defaultPath) const { return QFileDialog::getExistingDirectory(nullptr, "Select a directory", QString::fromStdString(defaultPath)); }
