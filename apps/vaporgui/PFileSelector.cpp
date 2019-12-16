#include "PFileSelector.h"
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <vapor/ParamsBase.h>
#include <vapor/FileUtils.h>

PFileSelector::PFileSelector(const std::string &tag, const std::string &label) : PLineItem(tag, _pathTexbox = new QLineEdit, _button = new QPushButton("Select"))
{
    _pathTexbox->setReadOnly(true);
    connect(_button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

void PFileSelector::updateGUI() const { _pathTexbox->setText(QString::fromStdString(getParams()->GetValueString(GetTag(), "<empty>"))); }

PFileSelector *PFileSelector::SetFileTypeFilter(const std::string &filter)
{
    _fileTypeFilter = QString::fromStdString(filter);
    return this;
}

void PFileSelector::buttonClicked()
{
    string defaultPath;
    string selectedFile = getParams()->GetValueString(GetTag(), "");

    if (Wasp::FileUtils::Exists(selectedFile))
        defaultPath = Wasp::FileUtils::Dirname(selectedFile);
    else
        defaultPath = Wasp::FileUtils::HomeDir();

    QString qSelectedPath = QFileDialog::getOpenFileName(nullptr, "Select a file", QString::fromStdString(defaultPath), _fileTypeFilter);
    if (qSelectedPath.isNull()) return;

    getParams()->SetValueString(GetTag(), GetTag(), qSelectedPath.toStdString());
}
