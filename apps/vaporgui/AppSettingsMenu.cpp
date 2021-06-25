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
#include "CheckForUpdate.h"
#include <QLabel>

class PUpdateChecker : public PWidget {
    VGroup *     _group;
    QLabel *     _label;
    VPushButton *_checkButton;
    VPushButton *_getButton;
    UpdateInfo   _updateInfo;

    void updateGUI() const override {}
    void openURL()
    {
        if (!_updateInfo.url.empty()) _updateInfo.OpenURL();
    }
    void checkForUpdate()
    {
        _label->setText("Checking...");
        _checkButton->setHidden(true);
        _label->setVisible(true);

        CheckForUpdate([=](bool updateAvailable, UpdateInfo info) {
            if (updateAvailable) {
                DisplayUpdateAvailable(info);
            } else {
                if (info.error)
                    DisplayError();
                else
                    DisplayUpToDate();
            }
        });
    }

public:
    PUpdateChecker() : PWidget("", _group = new VGroup)
    {
        _checkButton = new VPushButton("Check for update");
        _group->Add(_checkButton);

        _label = new QLabel;
        _label->setHidden(true);
        _group->Add(_label);

        _getButton = new VPushButton("Get Latest Version");
        _getButton->setHidden(true);
        _group->Add(_getButton);

        QObject::connect(_checkButton, &VPushButton::ButtonClicked, this, &PUpdateChecker::checkForUpdate);
        QObject::connect(_getButton, &VPushButton::ButtonClicked, this, &PUpdateChecker::openURL);
    }
    void DisplayUpToDate()
    {
        _label->setText("Vapor is up to date");

        _checkButton->setHidden(true);
        _label->setVisible(true);
        _getButton->setHidden(true);
    }
    void DisplayUpdateAvailable(UpdateInfo info)
    {
        _updateInfo = info;
        _label->setText(QString::fromStdString("New version available: " + info.version));

        _checkButton->setHidden(true);
        _label->setVisible(true);
        _getButton->setVisible(true);
    }
    void DisplayError()
    {
        _label->setText("Unable to check for new version.");

        _checkButton->setHidden(true);
        _label->setVisible(true);
        _getButton->setHidden(true);
    }
};

AppSettingsMenu::AppSettingsMenu(QWidget *parent) : QDialog(parent), Updateable(), _params(nullptr)
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
                         new PCheckboxHLI<SettingsParams>("Check for and show notices on startup", &SettingsParams::GetAutoCheckForNotices, &SettingsParams::SetAutoCheckForNotices),
                         new PCheckboxHLI<SettingsParams>("Automatically stretch domain", &SettingsParams::GetAutoStretchEnabled, &SettingsParams::SetAutoStretchEnabled),
                         new PCheckbox(SettingsParams::UseAllCoresTag, "Use all available cores for multithreaded tasks"),
                         new PSubGroup({(new PIntegerInputHLI<SettingsParams>("Limit threads to", &SettingsParams::GetNumThreads, &SettingsParams::SetNumThreads))
                                            ->SetRange(1, 1024)
                                            ->EnableBasedOnParam(SettingsParams::UseAllCoresTag, false)}),
                         new PIntegerInputHLI<SettingsParams>("Cache size (Megabytes)", &SettingsParams::GetCacheMB, &SettingsParams::SetCacheMB),
                         new PLabel("*Vapor must be restarted for these settings to take effect"),
                     }),

        new PSection("Default Search Paths", {new PDirectorySelectorHLI<SettingsParams>("Session file path", &SettingsParams::GetSessionDir, &SettingsParams::SetSessionDir),
                                              new PDirectorySelectorHLI<SettingsParams>("Data set path", &SettingsParams::GetMetadataDir, &SettingsParams::SetMetadataDir)}),

        // clang-format off
        new PSection("Updates", {
            new PCheckboxHLI<SettingsParams>("Automatically check for updates", &SettingsParams::GetAutoCheckForUpdates, &SettingsParams::SetAutoCheckForUpdates),
            new PUpdateChecker,
        }),
        // clang-format on

        new PButton("Restore defaults", [](VAPoR::ParamsBase *p) { dynamic_cast<SettingsParams *>(p)->Init(); }),
    });


    setLayout(new QVBoxLayout);
    layout()->addWidget(_settings);

    VPushButton *close = new VPushButton("Close");
    connect(close, &VPushButton::ButtonClicked, this, &AppSettingsMenu::accept);
    layout()->addWidget(close);

    setFocusPolicy(Qt::ClickFocus);
}

// Handle the pressing of the escape key
// Since settings are applied when changed in the gui,
// we still want to save the settings file, even if esc is pressed
void AppSettingsMenu::reject() { accept(); }

void AppSettingsMenu::accept()
{
    VAssert(_params != nullptr);
    int rc = _params->SaveSettings();
    if (rc < 0) {
        std::string settingsPath = _params->GetSettingsPath();
        MSG_ERR("Unable to write to settings file " + settingsPath);
    }
    QDialog::accept();
}

void AppSettingsMenu::Update(VAPoR::ParamsBase *p, VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr)
{
    _params = dynamic_cast<SettingsParams *>(p);
    VAssert(_params != nullptr);

    _settings->Update(_params);
}
