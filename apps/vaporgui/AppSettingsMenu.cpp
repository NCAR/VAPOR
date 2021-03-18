#include <QVBoxLayout>
#include <AppSettingsMenu.h>
#include <SettingsParams.h>
#include "PWidgets.h"
#include "PGroup.h"
#include "PFileSelectorHLI.h"
#include "PIntegerInputHLI.h"
#include "PCheckboxHLI.h"
#include "VPushButton.h"
#include "ErrorReporter.h"

AppSettingsMenu::AppSettingsMenu(QWidget *parent) : QDialog(parent), Updateable()
{
    _settings = new PGroup({
        new PSection("Automatic Session Recovery",
                     {
                         new PCheckboxHLI<SettingsParams>("Automatically save session", &SettingsParams::GetSessionAutoSaveEnabled, &SettingsParams::SetSessionAutoSaveEnabled),
                         (new PIntegerInputHLI<SettingsParams>("Changes per auto-save", &SettingsParams::GetChangesPerAutoSave, &SettingsParams::SetChangesPerAutoSave))
                             ->EnableBasedOnParam(SettingsParams::_sessionAutoSaveEnabledTag),
                         (new PFileSaveSelectorHLI<SettingsParams>("Auto-save file", &SettingsParams::GetAutoSaveSessionFile, &SettingsParams::SetAutoSaveSessionFile))
                             ->EnableBasedOnParam(SettingsParams::_sessionAutoSaveEnabledTag),
                     }),

        new PSection("Vapor's Startup Settings",
                     {
                         new PCheckboxHLI<SettingsParams>("Automatically stretch domain", &SettingsParams::GetAutoStretchEnabled, &SettingsParams::SetAutoStretchEnabled),
                         new PIntegerInputHLI<SettingsParams>("Number of threads (0 for available num. of CPU cores)", &SettingsParams::GetNumThreads, &SettingsParams::SetNumThreads),
                         new PIntegerInputHLI<SettingsParams>("Cache size (Megabytes)", &SettingsParams::GetCacheMB, &SettingsParams::SetCacheMB),
                         new PCheckboxHLI<SettingsParams>("Lock window dimensions", &SettingsParams::GetWinSizeLock, &SettingsParams::SetWinSizeLock),
                         (new PIntegerInputHLI<SettingsParams>("Width", &SettingsParams::GetWinWidth, &SettingsParams::SetWinWidth))->EnableBasedOnParam(SettingsParams::_winSizeLockTag),
                         (new PIntegerInputHLI<SettingsParams>("Height", &SettingsParams::GetWinHeight, &SettingsParams::SetWinHeight))->EnableBasedOnParam(SettingsParams::_winSizeLockTag),
                         new PLabel("*Vapor must be restarted for these settings to take effect"),
                     }),

        new PSection("Default Search Paths", {new PDirectorySelectorHLI<SettingsParams>("Session file path", &SettingsParams::GetSessionDir, &SettingsParams::SetSessionDir),
                                              new PDirectorySelectorHLI<SettingsParams>("Data set path", &SettingsParams::GetMetadataDir, &SettingsParams::SetMetadataDir)}),
        new PButton("Restore defaults", [](VAPoR::ParamsBase *p) { dynamic_cast<SettingsParams *>(p)->Init(); }),
    });


    setLayout(new QVBoxLayout);
    layout()->addWidget(_settings);

    VPushButton *close = new VPushButton("Close");
    connect(close, &VPushButton::ButtonClicked, this, &QDialog::accept);
    layout()->addWidget(close);

    setFocusPolicy(Qt::ClickFocus);
}

void AppSettingsMenu::Update(VAPoR::ParamsBase *p, VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr)
{
    SettingsParams *sp = dynamic_cast<SettingsParams *>(p);
    _settings->Update(sp);
    int rc = sp->SaveSettings();
    if (rc < 0) {
        std::string settingsPath = sp->GetSettingsPath();
        MSG_ERR("Unable to write to settings file " + settingsPath);
    }
}
