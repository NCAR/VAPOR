#include "CheckForUpdateUI.h"
#include "CheckForUpdate.h"
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <vapor/SettingsParams.h>
#include <vapor/ControlExecutive.h>


void CheckForAndShowUpdate(ControlExec *ce)
{
#ifndef NDEBUG
    return;    // Don't check for updates in debug builds
#endif

    CheckForUpdate([ce](bool updateAvailable, UpdateInfo info) {
        if (!updateAvailable) return;

        QCheckBox *cb = new QCheckBox("Automatically check for updates");
        cb->setChecked(true);
        QMessageBox popup;
        popup.setText(QString::fromStdString("A newer version of Vapor is available: " + info.version));
        QPushButton *get = popup.addButton("Get Latest Version", QMessageBox::ActionRole);
        popup.addButton("Ok", QMessageBox::AcceptRole);
        popup.setCheckBox(cb);

        popup.exec();
        if (popup.clickedButton() == get) info.OpenURL();

        if (!cb->isChecked()) {
            ce->GetParams<SettingsParams>()->SetValueLong(SettingsParams::AutoCheckForUpdatesTag, "", false);
            ce->GetParams<SettingsParams>()->SaveSettings();
        }
    });
}