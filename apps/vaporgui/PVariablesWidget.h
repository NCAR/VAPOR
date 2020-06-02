#pragma once

#include "PWidget.h"
#include "Flags.h"

#include "PVariableSelectorHLI.h"
#include "PLODSelectorHLI.h"

class VSection;
class VLineComboBox;
class VContainer;
class VFidelitySection_PW;

//! \class PVariablesWidget
//! A PWidget that encapsulates Variable and Fidelity selectors.
//! These Variable and Fidelity selectors are implemented as
//! PWidgets wherever possible, and as VWidgets elsewhere.

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

    std::vector<PWidget*>    _pWidgetVec;

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
};
