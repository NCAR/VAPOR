#pragma once

#include <QDialog>
#include "PWidget.h"
#include "PFileSelectorHLI.h"


class QWidget;
class SettingsParams;
class PGroup;
class PSection;

//! \class Preferences Menu
//! \ingroup Public_GUI
//! \brief Menu for global application preferences
//! \author Scott Pearse

//class AppSettingsMenu : public QDialog {
class AppSettingsMenu : public QWidget {
    Q_OBJECT

public:
    AppSettingsMenu( QWidget* parent );
    //AppSettingsMenu();
    void Update( SettingsParams* sp );

public slots:
    void ShowEvent();

private:
    PFileSaveSelectorHLI<SettingsParams> *pfile;
    //PFileOpenSelectorHLI<SettingsParams> *pfile;
    PGroup* _settings;
};
