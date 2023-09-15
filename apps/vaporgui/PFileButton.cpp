#include "PFileButton.h"
#include "VPushButton.h"
#include <QFileDialog>
#include <vapor/ParamsBase.h>
#include <vapor/FileUtils.h>

PFileButton::PFileButton(const std::string label, Callback cb) : PWidget("", _button= new VPushButton(label)), _cb(cb)
{
    QObject::connect(_button, &VPushButton::ButtonClicked, this, &PFileButton::clicked);
}

void PFileButton::updateGUI() const {}

PFileButton *PFileButton::SetFileTypeFilter(const std::string &filter)
{
    _fileTypeFilter = QString::fromStdString(filter);
    return this;
}

void PFileButton::clicked()
{
    std::string defaultPath = Wasp::FileUtils::HomeDir();

    QString qSelectedPath = selectPath(defaultPath);
    if (qSelectedPath.isNull()) return;

    _cb(qSelectedPath.toStdString());
}

bool PFileButton::requireParamsMgr() const { return false; }

QString PFileWriter::selectPath(const std::string &defaultPath) const { return QFileDialog::getSaveFileName(nullptr, "Select a file", QString::fromStdString(defaultPath), _fileTypeFilter); }

QString PFileReader::selectPath(const std::string &defaultPath) const { return QFileDialog::getOpenFileName(nullptr, "Select a save file", QString::fromStdString(defaultPath), _fileTypeFilter); }
