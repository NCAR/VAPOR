#pragma once

#include "PWidget.h"
#include "VSection.h"
#include "Flags.h"

#include "PVariableSelectorHLI.h"

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

//class PVariableSelector;
//class VComboBox;
class VLineComboBox;
class VContainer;
class PFidelityWidget3;
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
    size_t _activeDim;
    bool   _initialized;

    VariableFlags _variableFlags;
    DimFlags      _dimFlags;

    VSection* _vSection;
    VLineComboBox* _dimCombo;

    VContainer* _container;

    PVariableSelector3DHLI<VAPoR::RenderParams>* _pvshli;

    // We cannot hide PVariableSelector, so use
    // VLineComboBox and signal/slot connections
    VLineComboBox* _scalarCombo;
    VLineComboBox* _xFieldCombo;
    VLineComboBox* _yFieldCombo;
    VLineComboBox* _zFieldCombo;
    VLineComboBox* _colorCombo;
    VLineComboBox* _heightCombo;

    PFidelityWidget3* _fidelityWidget;

    static const std::string _sectionTitle;

private slots:
    void _dimChanged();
    void _scalarVarChanged( std::string var );
    void _xFieldVarChanged( std::string var );
    void _yFieldVarChanged( std::string var );
    void _zFieldVarChanged( std::string var );
    void _colorVarChanged(  std::string var );
    void _heightVarChanged( std::string var );
};
