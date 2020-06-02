#include "vapor/RenderParams.h"

#include "PVariablesWidget.h"
#include "VFidelitySection_PW.h"
#include "VLineComboBox.h"
#include "VContainer.h"
#include "VSection.h"

#include <QLayout>
#include <QLabel>
#include <QSpacerItem>

const std::string PVariablesWidget::_sectionTitle = "Variable Selection";
    
// The MAKE_COMPILER_ERROR macro will swap the setter and getter functions for 
// _pscalarHLI3d, causing the compilation to fail.  This error (shown below) 
// was hard to decipher.
//
//
// MAKE_COMPILER_ERROR:
//   /Users/pearse/VAPOR/apps/vaporgui/PVariablesWidget.cpp:34:25: error: no matching constructor for
//         initialization of 'PVariableSelector3DHLI<VAPoR::RenderParams>'
//       _pscalarHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>(
//                           ^
//   /Users/pearse/VAPOR/apps/vaporgui/PVariableSelectorHLI.h:61:5: note: candidate constructor not viable: no
//         known conversion from 'void (VAPoR::RenderParams::*)(std::__1::string)' to 'typename
//         PWidgetHLIBase<RenderParams, std::string>::GetterType' (aka 'function<basic_string<char,
//         char_traits<char>, allocator<char> > (VAPoR::RenderParams *)>') for 2nd argument
//       PVariableSelector3DHLI(
//       ^
//   /Users/pearse/VAPOR/apps/vaporgui/PVariableSelectorHLI.h:55:7: note: candidate constructor
//         (the implicit copy constructor) not viable: requires 1 argument, but 3 were provided
//   class PVariableSelector3DHLI :
//         ^
//   1 error generated.
//   make[2]: *** [apps/vaporgui/CMakeFiles/vapor.dir/PVariablesWidget.cpp.o] Error 1
//   make[1]: *** [apps/vaporgui/CMakeFiles/vapor.dir/all] Error 2
//   make: *** [all] Error 2
//
//
//
// Normal type mismatch error (see line 82):
//    /Users/pearse/VAPOR/apps/vaporgui/VFidelitySection_PW.cpp:82:26: error: cannot initialize a parameter of
//          type 'QWidget *' with an rvalue of type 'int'
//        layout()->addWidget( (int)1 );
//                             ^~~~~~
//    /usr/local/VAPOR-Deps/2019-Aug/Qt/5.13.2/clang_64/lib/QtWidgets.framework/Headers/qboxlayout.h:74:29: note:
//          passing argument to parameter here
//        void addWidget(QWidget *, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
//                                  ^
//    /Users/pearse/VAPOR/apps/vaporgui/PVariablesWidget.cpp:61:21: error: no matching constructor for
//          initialization of 'VSection'
//        _vSection = new VSection( (int)1 );
//                          ^
//    /Users/pearse/VAPOR/apps/vaporgui/VSection.h:16:7: note: candidate constructor
//          (the implicit copy constructor) not viable: no known conversion from 'int' to 'const VSection' for
//          1st argument
//    class VSection : public QTabWidget {
//          ^
//    /Users/pearse/VAPOR/apps/vaporgui/VSection.h:22:5: note: candidate constructor not viable: no known
//          conversion from 'int' to 'const std::string' (aka 'const basic_string<char, char_traits<char>,
//          allocator<char> >') for 1st argument
//        VSection(const std::string &title);
//        ^
//    1 error generated.
//    make[2]: *** [apps/vaporgui/CMakeFiles/vapor.dir/VFidelitySection_PW.cpp.o] Error 1
//    make[2]: *** Waiting for unfinished jobs....
//    1 error generated.
//    make[2]: *** [apps/vaporgui/CMakeFiles/vapor.dir/PVariablesWidget.cpp.o] Error 1
//    make[1]: *** [apps/vaporgui/CMakeFiles/vapor.dir/all] Error 2
//    make: *** [all] Error 2
//
//#define MAKE_COMPILER_ERROR
//

PVariablesWidget::PVariablesWidget() 
    : PWidget( "", _container = new VContainer( new QVBoxLayout ) ),
    _activeDim( 3 ),
    _initialized( false )
{
    // Uncomment this line to reproduce the "Normal type mismatch error, shown above"
    //_vSection = new VSection( (int)1 );

    _vSection = new VSection( "Variable Selection" );
    _container->layout()->addWidget( _vSection );

    _dimCombo = new VLineComboBox( "Variable Dimension" );
    _dimCombo->SetOptions( {"3D", "2D"} );
    _vSection->layout()->addWidget( _dimCombo );
    connect( _dimCombo, &VLineComboBox::ValueChanged,
        this, &PVariablesWidget::_dimChanged );
    
    _pscalarHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "Variable name",
#ifdef MAKE_COMPILER_ERROR
        &VAPoR::RenderParams::SetVariableName,
        &VAPoR::RenderParams::GetVariableName
#else
        &VAPoR::RenderParams::GetVariableName,
        &VAPoR::RenderParams::SetVariableName
#endif
    );
    _pWidgetVec.push_back( _pscalarHLI3D );
    _pscalarHLIContainer3D = new VContainer();
    _pscalarHLIContainer3D->layout()->addWidget( _pscalarHLI3D );
    _vSection->layout()->addWidget( _pscalarHLIContainer3D );
  
    _pXFieldHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "  X",
        &VAPoR::RenderParams::GetXFieldVariableName,
        &VAPoR::RenderParams::SetXFieldVariableName
    );
    _pWidgetVec.push_back( _pXFieldHLI3D );
    _pXFieldHLIContainer3D = new VContainer();
    _pXFieldHLIContainer3D->layout()->addWidget( _pXFieldHLI3D );
    _vSection->layout()->addWidget( _pXFieldHLIContainer3D );

    _pYFieldHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "  Y",
        &VAPoR::RenderParams::GetYFieldVariableName,
        &VAPoR::RenderParams::SetYFieldVariableName
    );
    _pWidgetVec.push_back( _pYFieldHLI3D );
    _pYFieldHLIContainer3D = new VContainer();
    _pYFieldHLIContainer3D->layout()->addWidget( _pYFieldHLI3D );
    _vSection->layout()->addWidget( _pYFieldHLIContainer3D );

    _pZFieldHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "  Z",
        &VAPoR::RenderParams::GetZFieldVariableName,
        &VAPoR::RenderParams::SetZFieldVariableName
    );
    _pWidgetVec.push_back( _pZFieldHLI3D );
    _pZFieldHLIContainer3D = new VContainer();
    _pZFieldHLIContainer3D->layout()->addWidget( _pZFieldHLI3D );
    _vSection->layout()->addWidget( _pZFieldHLIContainer3D );

    _pheightHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "Color variable",
        &VAPoR::RenderParams::GetHeightVariableName,
        &VAPoR::RenderParams::SetHeightVariableName
    );
    _pWidgetVec.push_back( _pheightHLI2D );
    _pheightHLIContainer2D = new VContainer();
    _pheightHLIContainer2D->layout()->addWidget( _pheightHLI2D );
    _vSection->layout()->addWidget( _pheightHLIContainer2D );

    _pcolorHLI3D = new PVariableSelector3DHLI<VAPoR::RenderParams>( 
        "Color variable",
        &VAPoR::RenderParams::GetColorMapVariableName,
        &VAPoR::RenderParams::SetColorMapVariableName
    );
    _pWidgetVec.push_back( _pcolorHLI3D );
    _pcolorHLIContainer3D = new VContainer();
    _pcolorHLIContainer3D->layout()->addWidget( _pcolorHLI3D );
    _vSection->layout()->addWidget( _pcolorHLIContainer3D );

    _pscalarHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "Variable name",
        &VAPoR::RenderParams::GetVariableName,
        &VAPoR::RenderParams::SetVariableName
    );
    _pWidgetVec.push_back( _pscalarHLI2D );
    _pscalarHLIContainer2D = new VContainer();
    _pscalarHLIContainer2D->layout()->addWidget( _pscalarHLI2D );
    _vSection->layout()->addWidget( _pscalarHLIContainer2D );
  
    _pXFieldHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "  X",
        &VAPoR::RenderParams::GetXFieldVariableName,
        &VAPoR::RenderParams::SetXFieldVariableName
    );
    _pWidgetVec.push_back( _pXFieldHLI2D );
    _pXFieldHLIContainer2D = new VContainer();
    _pXFieldHLIContainer2D->layout()->addWidget( _pXFieldHLI2D );
    _vSection->layout()->addWidget( _pXFieldHLIContainer2D );

    _pYFieldHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "  Y",
        &VAPoR::RenderParams::GetYFieldVariableName,
        &VAPoR::RenderParams::SetYFieldVariableName
    );
    _pWidgetVec.push_back( _pYFieldHLI2D );
    _pYFieldHLIContainer2D = new VContainer();
    _pYFieldHLIContainer2D->layout()->addWidget( _pYFieldHLI2D );
    _vSection->layout()->addWidget( _pYFieldHLIContainer2D );

    _pZFieldHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "  Z",
        &VAPoR::RenderParams::GetZFieldVariableName,
        &VAPoR::RenderParams::SetZFieldVariableName
    );
    _pWidgetVec.push_back( _pZFieldHLI2D );
    _pZFieldHLIContainer2D = new VContainer();
    _pZFieldHLIContainer2D->layout()->addWidget( _pZFieldHLI2D );
    _vSection->layout()->addWidget( _pZFieldHLIContainer2D );

    _pcolorHLI2D = new PVariableSelector2DHLI<VAPoR::RenderParams>( 
        "Color variable",
        &VAPoR::RenderParams::GetColorMapVariableName,
        &VAPoR::RenderParams::SetColorMapVariableName
    );
    _pWidgetVec.push_back( _pcolorHLI2D );
    _pcolorHLIContainer2D = new VContainer();
    _pcolorHLIContainer2D->layout()->addWidget( _pcolorHLI2D );
    _vSection->layout()->addWidget( _pcolorHLIContainer2D );

    _fidelityWidget = new VFidelitySection_PW();
    _container->layout()->addWidget( _fidelityWidget );

    _container->layout()->addItem( 
        new QSpacerItem( 1, 2000, QSizePolicy::Minimum, QSizePolicy::Maximum ) 
    );
}

void PVariablesWidget::Reinit(
    VariableFlags variableFlags,
    DimFlags dimFlags
) {
    _variableFlags = variableFlags;
    _dimFlags = dimFlags;

    // If the renderer is not both 2D and 3D, hide
    // the dimension selector and set the _activeDim
    if (! ( ( _dimFlags & 2 ) && ( _dimFlags & 3 ) )
    ) { 
        _dimCombo->hide();
        if (dimFlags & THREED) {
            _activeDim = 3;
        }
        else {
            _activeDim = 2;
        }
    }

    if ( _activeDim == 2 ) {
        _pscalarHLIContainer3D->hide();
        _pXFieldHLIContainer3D->hide();
        _pYFieldHLIContainer3D->hide();
        _pZFieldHLIContainer3D->hide();
        _pcolorHLIContainer3D->hide();
        if (_variableFlags & SCALAR) {
            _pscalarHLIContainer2D->show();
        }
        else {
            _pscalarHLIContainer2D->hide();
        }

        if ( _variableFlags & VECTOR ) {
            _pXFieldHLIContainer2D->show();
            _pYFieldHLIContainer2D->show();
            _pZFieldHLIContainer2D->show();
        }
        else {
            _pXFieldHLIContainer2D->hide();
            _pYFieldHLIContainer2D->hide();
            _pZFieldHLIContainer2D->hide();
        }

        if ( _variableFlags & HEIGHT ) {
            _pheightHLIContainer2D->show();
        }
        else {
            _pheightHLIContainer2D->hide();
        }

        if ( _variableFlags & COLOR ) {
            _pcolorHLIContainer2D->show();
        }
        else {
            _pcolorHLIContainer2D->hide();
        }
    }
    else {
        _pscalarHLIContainer2D->hide();
        _pXFieldHLIContainer2D->hide();
        _pYFieldHLIContainer2D->hide();
        _pZFieldHLIContainer2D->hide();
        _pcolorHLIContainer2D->hide();
        if (_variableFlags & SCALAR) {
            _pscalarHLIContainer3D->show();
        }
        else {
            _pscalarHLIContainer3D->hide();
        }

        if ( _variableFlags & VECTOR ) {
            _pXFieldHLIContainer3D->show();
            _pYFieldHLIContainer3D->show();
            _pZFieldHLIContainer3D->show();
        }
        else {
            _pXFieldHLIContainer3D->hide();
            _pYFieldHLIContainer3D->hide();
            _pZFieldHLIContainer3D->hide();
        }

        if ( _variableFlags & HEIGHT ) {
            _pheightHLIContainer2D->show();
        }
        else {
            _pheightHLIContainer2D->hide();
        }

        if ( _variableFlags & COLOR ) {
            _pcolorHLIContainer3D->show();
        }
        else {
            _pcolorHLIContainer3D->hide();
        }
    }

    _fidelityWidget->Reinit( _variableFlags );
}

void PVariablesWidget::updateGUI() const {
   
    VAPoR::DataMgr* dataMgr = getDataMgr(); 
    std::vector<std::string> twoDVars = dataMgr->GetDataVarNames( 2 );

    std::vector<std::string> activeVars;
    if ( _activeDim == 3 ) {
        activeVars = dataMgr->GetDataVarNames( 3 );
    }
    else {
        activeVars = twoDVars;
    }

    VAPoR::ParamsBase* params = getParams();
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( params );
    VAPoR::ParamsMgr*  paramsMgr = getParamsMgr();
    _fidelityWidget->Update( rParams, paramsMgr, dataMgr );

    //
    // These widgets must be updated manually because they cannot
    // be added to a PGroup, which would otherwise update them.
    //
    // They cannot be added to a PGroup becuase they need to be
    // wrapped in a VContainer in order to be shown/hidden.
    //
    for( PWidget* pw : _pWidgetVec ) {
        pw->Update( rParams, paramsMgr, dataMgr );
    }
};

void PVariablesWidget::_dimChanged() {
    VAPoR::ParamsBase* params = getParams();
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( params );

    // Index 0 is 3D, 1 is 2D
    _activeDim = _dimCombo->GetCurrentIndex() == 0 ? 3 : 2;

    rParams->SetDefaultVariables( _activeDim, false );
}

int PVariablesWidget::GetActiveDimension() const {
    return _activeDim;
}

DimFlags PVariablesWidget::GetDimFlags() const {
    return _dimFlags;
}

void PVariablesWidget::Configure2DFieldVars() {
    VAPoR::ParamsBase* params = getParams();
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( params );
    rParams->SetDefaultVariables( 2, false );
}

void PVariablesWidget::Configure3DFieldVars() {
    VAPoR::ParamsBase* params = getParams();
    VAPoR::RenderParams* rParams = dynamic_cast< VAPoR::RenderParams* >( params );
    rParams->SetDefaultVariables( 3, false );
}
