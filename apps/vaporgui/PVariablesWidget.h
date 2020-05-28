#pragma once

#include "PWidget.h"
#include "VSection.h"
#include "Flags.h"

#include "PVariableSelectorHLI.h"
#include "PLODSelectorHLI.h"

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

//class PVariableSelector;
//class VComboBox;
class VLineComboBox;
class VContainer;
class VFidelitySection_PW;
class PFidelityWidget3;
class FidelityWidget3;
class PSection;
//class VLineItem;
//class FidelityWidget2;

class PVariablesWidget : public PWidget {

    Q_OBJECT

public:

    PVariablesWidget();
    void Reinit(
        VariableFlags varFlags,
        DimFlags      dimFlags
    );

    int GetActiveDimension() const;
    DimFlags GetDimFlags() const;

    void Configure2DFieldVars();
    void Configure3DFieldVars();

protected:
    VAPoR::RenderParams* _rParams;

    void updateGUI() const override;

private:
    size_t                   _activeDim;
    bool                     _initialized;
    VariableFlags            _variableFlags;
    DimFlags                 _dimFlags;

    static const std::string _sectionTitle;

    VSection*                _vSection;
    VContainer*              _container;
    VLineComboBox*           _dimCombo;

    PVariableSelector2DHLI<VAPoR::RenderParams>* _pscalarHLI2D;
    PVariableSelector3DHLI<VAPoR::RenderParams>* _pscalarHLI3D;
    PVariableSelector2DHLI<VAPoR::RenderParams>* _pXFieldHLI2D;
    PVariableSelector3DHLI<VAPoR::RenderParams>* _pXFieldHLI3D;
    PVariableSelector2DHLI<VAPoR::RenderParams>* _pYFieldHLI2D;
    PVariableSelector3DHLI<VAPoR::RenderParams>* _pYFieldHLI3D;
    PVariableSelector2DHLI<VAPoR::RenderParams>* _pZFieldHLI2D;
    PVariableSelector3DHLI<VAPoR::RenderParams>* _pZFieldHLI3D;
    PVariableSelector2DHLI<VAPoR::RenderParams>* _pheightHLI2D;
    PVariableSelector2DHLI<VAPoR::RenderParams>* _pcolorHLI2D;
    PVariableSelector3DHLI<VAPoR::RenderParams>* _pcolorHLI3D;

    // VContainers to encapsulate the PVariableSelectorHLIs,
    // so we can hide them
    VContainer*                                  _pscalarHLIContainer2D;
    VContainer*                                  _pscalarHLIContainer3D;
    VContainer*                                  _pXFieldHLIContainer2D;
    VContainer*                                  _pXFieldHLIContainer3D;
    VContainer*                                  _pYFieldHLIContainer2D;
    VContainer*                                  _pYFieldHLIContainer3D;
    VContainer*                                  _pZFieldHLIContainer2D;
    VContainer*                                  _pZFieldHLIContainer3D;
    VContainer*                                  _pheightHLIContainer2D;
    VContainer*                                  _pcolorHLIContainer2D;
    VContainer*                                  _pcolorHLIContainer3D;
    
    VFidelitySection_PW*                         _fidelityWidget;

private slots:
    void _dimChanged();
    void _scalarVarChanged( std::string var );
    void _xFieldVarChanged( std::string var );
    void _yFieldVarChanged( std::string var );
    void _zFieldVarChanged( std::string var );
    void _colorVarChanged(  std::string var );
    void _heightVarChanged( std::string var );
};
