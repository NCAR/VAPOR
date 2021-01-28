#pragma once

#include <QDialog>

class QWidget;
class SettingsParams;

//! \class Preferences Menu
//! \ingroup Public_GUI
//! \brief Menu for global application preferences
//! \author Scott Pearse

class PreferencesMenu : public QDialog {
public:
    PreferencesMenu( QWidget* parent );
    void Update( SettingsParams* sp );
};
