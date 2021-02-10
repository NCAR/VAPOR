#include <QVBoxLayout>
#include <AppSettingsMenu.h>
#include <SettingsParams.h>
#include "PWidgets.h"
#include "PGroup.h"
#include "PFileSelector.h"
#include "PFileSelectorHLI.h"
#include "PIntegerInputHLI.h"
#include "PCheckboxHLI.h"

//AppSettingsMenu::AppSettingsMenu()
//AppSettingsMenu::AppSettingsMenu() : QDialog()
//AppSettingsMenu::AppSettingsMenu(QWidget *parent) : QDialog( parent )
AppSettingsMenu::AppSettingsMenu(QWidget *parent) : QWidget( parent )
{
    setWindowModality(Qt::WindowModal);
    std::cout << "AppSettingsMenu constructor" << std::endl;
    pfile = new PFileSaveSelectorHLI<SettingsParams>( 
            "Auto-save open session file", 
            &SettingsParams::GetAutoSaveSessionFile, 
            &SettingsParams::SetAutoSaveSessionFile
        ),

    _settings = new PGroup( {
            new PCheckboxHLI<SettingsParams>( 
                "Automatically stretch domain", 
                &SettingsParams::GetAutoStretchEnabled, 
                &SettingsParams::SetAutoStretchEnabled
            ),


    /*_generalSettings = new PSection("General Settings", {
        new PCheckboxHLI<SettingsParams>( 
            "Automatically stretch domain", 
            &SettingsParams::GetAutoStretchEnabled, 
            &SettingsParams::SetAutoStretchEnabled
        ),*/
        /*new PFileSaveSelectorHLI<SettingsParams>( 
            "Auto-save session file", 
            &SettingsParams::GetAutoSaveSessionFile, 
            &SettingsParams::SetAutoSaveSessionFile
        ),*/

        /*new PSection("General Settings", {
            new PIntegerInputHLI<SettingsParams>( 
                "Changes per save", 
                &SettingsParams::GetChangesPerAutoSave, 
                &SettingsParams::SetChangesPerAutoSave
            ),
            new PCheckboxHLI<SettingsParams>( 
                "Automatically stretch domain", 
                &SettingsParams::GetAutoStretchEnabled, 
                &SettingsParams::SetAutoStretchEnabled
            ),
            new PFileSaveSelectorHLI<SettingsParams>( 
                "Auto-save session file", 
                &SettingsParams::GetAutoSaveSessionFile, 
                &SettingsParams::SetAutoSaveSessionFile
            ),
            
        } ),*/
        

        /*new PSection("Startup Settings", {
            new PCheckboxHLI<SettingsParams>( 
                "Automatically stretch domain", 
                &SettingsParams::GetAutoStretchEnabled, 
                &SettingsParams::SetAutoStretchEnabled
            ), 
            new PIntegerInputHLI<SettingsParams>( 
                "Number of threads (0 for available num. of CPU cores)", 
                &SettingsParams::GetNumThreads, 
                &SettingsParams::SetNumThreads
            ),
            new PIntegerInputHLI<SettingsParams>( 
                "Cache size (Megabytes)", 
                &SettingsParams::GetCacheMB, 
                &SettingsParams::SetCacheMB
            ),
            //new PCheckbox( 
            //    SettingsParams::_winSizeLockTag, 
            //    "Lock window dimensions"
            //),
            new PCheckboxHLI<SettingsParams>(
                "Lock window dimensions",
                &SettingsParams::GetWinSizeLock,
                &SettingsParams::SetWinSizeLock
            ),
            (new PIntegerInputHLI<SettingsParams>(
                "Width",
                &SettingsParams::GetWinWidth,
                &SettingsParams::SetWinWidth
            ))->EnableBasedOnParam( SettingsParams::_winSizeLockTag ),
            (new PIntegerInputHLI<SettingsParams>(
                "Height",
                &SettingsParams::GetWinHeight,
                &SettingsParams::SetWinHeight
            ))->EnableBasedOnParam( SettingsParams::_winSizeLockTag )
            
            //(new PCheckboxHLI<SettingsParams>( 
            //    "Lock window dimensions", 
            //    &SettingsParams::SetWinSizeLock, 
            //    &SettingsParams::GetWinSizeLock)
            //),
            //(new PIntegerInput( SettingsParams::_changesPerAutoSaveTag, "Number of threads (0 for available num. of CPU cores)")),
            
        } ),*/
        
        /*new PSection("Default Search Paths", {
            new PDirectorySelectorHLI<SettingsParams>( 
                "Session file path", 
                &SettingsParams::GetSessionDir, 
                &SettingsParams::SetSessionDir
            ),
            new PDirectorySelectorHLI<SettingsParams>( 
                "Data set path", 
                &SettingsParams::GetMetadataDir, 
                &SettingsParams::SetMetadataDir
            )
        } ),
        new PButton( "Apply settings",  [](VAPoR::ParamsBase *p){
            dynamic_cast<SettingsParams*>(p)->SaveSettings();
        })*/
    } );

    setLayout( new QVBoxLayout );
    layout()->addWidget( pfile );
    layout()->addWidget( _settings );
    
}

void AppSettingsMenu::ShowEvent() {
    show();
    raise();
}

void AppSettingsMenu::Update( SettingsParams* sp ) {
    std::cout << sp << " " << sp->GetFontSize() << std::endl;
    pfile->Update( sp );
    _settings->Update( sp );
    std::cout << "AppSettingsMenu::Update( SettingsParams* sp )" << std::endl;
}
