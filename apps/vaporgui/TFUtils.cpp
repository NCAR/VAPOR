#include "TFUtils.h"
#include <vapor/MapperFunction.h>
#include <vapor/FileUtils.h>
#include <vapor/ParamsMgr.h>
#include "SettingsParams.h"
#include "ErrorReporter.h"
#include <QFileDialog>
#include <cassert>

void TFUtils::LoadColormap(VAPoR::MapperFunction *tf, const std::string &path)
{
    int rc = tf->LoadColormapFromFile(path);

    if (rc < 0) MSG_ERR("Failed to load transfer function");
}

void TFUtils::LoadColormap(VAPoR::ParamsMgr *paramsMgr, VAPoR::MapperFunction *tf)
{
    assert(paramsMgr);

    SettingsParams *sp = (SettingsParams *)paramsMgr->GetParams(SettingsParams::GetClassType());

    QString qDefaultDirectory = QString::fromStdString(sp->GetSessionDir());
    QString qSelectedPath = QFileDialog::getOpenFileName(nullptr, "Select a .tf3 file", qDefaultDirectory, "Vapor Transfer Function (*.tf3)");
    if (qSelectedPath.isNull()) return;

    string selectedPath = qSelectedPath.toStdString();
    sp->SetSessionDir(Wasp::FileUtils::Dirname(selectedPath));

    LoadColormap(tf, selectedPath);
}

void TFUtils::LoadTransferFunction(VAPoR::ParamsMgr *paramsMgr, VAPoR::MapperFunction *tf)
{
    SettingsParams *sp = (SettingsParams *)paramsMgr->GetParams(SettingsParams::GetClassType());

    QString qDefaultDirectory = QString::fromStdString(sp->GetSessionDir());
    QString qSelectedPath = QFileDialog::getOpenFileName(nullptr, "Select a .tf3 file", qDefaultDirectory, "Vapor Transfer Function (*.tf3)");
    if (qSelectedPath.isNull()) return;

    string selectedPath = qSelectedPath.toStdString();
    sp->SetSessionDir(FileUtils::Dirname(selectedPath));

    int rc = tf->LoadFromFile(selectedPath);

    if (rc < 0) MSG_ERR("Failed to load transfer function");
}

void TFUtils::SaveTransferFunction(VAPoR::ParamsMgr *paramsMgr, VAPoR::MapperFunction *tf)
{
    SettingsParams *sp = (SettingsParams *)paramsMgr->GetParams(SettingsParams::GetClassType());

    QString qDefaultDirectory = QString::fromStdString(sp->GetSessionDir());
    QString qSelectedPath = QFileDialog::getSaveFileName(nullptr, "Select a .tf3 file", qDefaultDirectory, "Vapor Transfer Function (*.tf3)", 0);
    if (qSelectedPath.isNull()) return;

    string selectedPath = qSelectedPath.toStdString();
    sp->SetSessionDir(FileUtils::Dirname(selectedPath));

    int rc = tf->SaveToFile(selectedPath);

    if (rc < 0) MSG_ERR("Failed to save transfer function");
}
