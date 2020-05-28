#pragma once

#include "VSection.h"
#include "Flags.h"

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

class VLineComboBox;
class VContainer;

class VVariablesSection : public VSection {

    Q_OBJECT

public:

    VVariablesSection();

    void Reinit(
        VariableFlags varFlags,
        DimFlags      dimFlags
    );

    int GetActiveDimension() const;
    DimFlags GetDimFlags() const;

    void Configure2DFieldVars();
    void Configure3DFieldVars();

    void Update(
        VAPoR::RenderParams* rParams,
        VAPoR::ParamsMgr*    paramsMgr,
        VAPoR::DataMgr*      dataMgr
    );

protected:
    VAPoR::RenderParams* _rParams;
    VAPoR::ParamsMgr*    _paramsMgr;
    VAPoR::DataMgr*      _dataMgr;

private:
    size_t _activeDim;
    bool   _initialized;

    VariableFlags _variableFlags;
    DimFlags      _dimFlags;

    VSection* _vSection;
    VLineComboBox* _dimCombo;

    VLineComboBox* _scalarCombo;
    VLineComboBox* _xFieldCombo;
    VLineComboBox* _yFieldCombo;
    VLineComboBox* _zFieldCombo;
    VLineComboBox* _colorCombo;
    VLineComboBox* _heightCombo;

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
