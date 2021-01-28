#include <PreferencesMenu.h>
#include <SettingsParams.h>
#include "PWidgets.h"

PreferencesMenu::PreferencesMenu(QWidget *parent) : QDialog( parent ) {
    std::cout << "PreferencesMenu constructor" << std::endl;
}

void PreferencesMenu::Update( SettingsParams* sp ) {
    std::cout << "PreferencesMenu::Update( SettingsParams* sp )" << std::endl;
}
