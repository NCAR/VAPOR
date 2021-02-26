#pragma once

#include <QDialog>
#include "PWidget.h"
#include "PFileSelectorHLI.h"
#include "VFileSelector.h"

class QWidget;
class SettingsParams;
class PGroup;
class PSection;

//! \class Preferences Menu
//! \ingroup Public_GUI
//! \brief Menu for global application preferences
//! \author Scott Pearse

class AppSettingsMenu : public QDialog {
    Q_OBJECT

public:
    AppSettingsMenu(QWidget *parent);
    void Update(SettingsParams *sp);

private:
    PGroup *_settings;
};
